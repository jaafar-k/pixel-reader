#include "./reader_view.h"

#include "./popup_view.h"
#include "./selection_menu.h"
#include "./token_view/token_view.h"
#include "./token_view/token_view_styling.h"

#include "reader/config.h"
#include "reader/shoulder_keymap.h"
#include "reader/state_store.h"
#include "reader/system_styling.h"
#include "reader/text_direction.h"
#include "reader/view_stack.h"

#include "doc_api/doc_reader.h"
#include "sys/keymap.h"
#include "sys/screen.h"
#include "util/sdl_font_cache.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>

struct ReaderViewState
{
    bool is_done = false;

    std::function<void(DocAddr)> on_change_address;

    std::string filename;
    std::shared_ptr<DocReader> reader;
    SystemStyling &sys_styling;
    TokenViewStyling &token_view_styling;
    uint32_t token_view_styling_sub_id;

    ViewStack &view_stack;
    StateStore &state_store;
    std::string book_id;

    std::unique_ptr<TokenView> token_view;

    ReaderViewState(
        std::filesystem::path path,
        DocAddr seek_address,
        std::shared_ptr<DocReader> reader,
        SystemStyling &sys_styling,
        TokenViewStyling &token_view_styling,
        uint32_t token_view_styling_sub_id,
        ViewStack &view_stack,
        StateStore &state_store,
        std::string book_id
    )
        : filename(path.filename()),
          reader(reader),
          sys_styling(sys_styling),
          token_view_styling(token_view_styling),
          token_view_styling_sub_id(token_view_styling_sub_id),
          view_stack(view_stack),
          state_store(state_store),
          book_id(std::move(book_id)),
          token_view(std::make_unique<TokenView>(
              reader,
              seek_address,
              sys_styling,
              token_view_styling
          ))
    {
    }

    ~ReaderViewState()
    {
    }
};

namespace
{

DocAddr get_current_address(const ReaderViewState &state)
{
    if (state.token_view)
    {
        return state.token_view->get_address();
    }

    return 0;
}

std::string make_bookmark_label(const ReaderViewState &state, DocAddr address)
{
    const auto &toc = state.reader->get_table_of_contents();
    auto toc_position = state.reader->get_toc_position(address);
    uint32_t progress_percent = state.reader->get_global_progress_percent(address);

    std::string title = state.filename;
    if (toc_position.toc_index < toc.size())
    {
        title = toc[toc_position.toc_index].display_name;
    }

    return title + " · " + std::to_string(progress_percent) + "%";
}

std::vector<std::string> bookmark_menu_names(const std::vector<Bookmark> &bookmarks)
{
    std::vector<std::string> names;
    names.reserve(bookmarks.size());
    for (const auto &bm : bookmarks)
    {
        names.push_back(bm.label.empty() ? encode_address(bm.address) : bm.label);
    }
    return names;
}

bool bookmarks_rtl(const ReaderViewState &state, const std::vector<Bookmark> &bookmarks)
{
    std::string sample;
    for (const auto &bm : bookmarks)
    {
        sample += bm.label;
        sample.push_back(' ');
        if (sample.size() > 512)
        {
            break;
        }
    }
    return resolve_effective_rtl(
        state.sys_styling.get_text_direction(),
        state.reader->get_language(),
        sample
    );
}

void open_toc_menu(ReaderView &reader_view, ReaderViewState &state)
{
    const auto &toc = state.reader->get_table_of_contents();
    if (toc.empty())
    {
        return;
    }

    // Sample TOC labels for Auto direction detection
    std::string toc_sample;
    for (const auto &toc_item: toc)
    {
        toc_sample += toc_item.display_name;
        toc_sample.push_back(' ');
        if (toc_sample.size() > 512)
        {
            break;
        }
    }
    bool toc_rtl = resolve_effective_rtl(
        state.sys_styling.get_text_direction(),
        state.reader->get_language(),
        toc_sample
    );

    // setup toc entries & callbacks
    std::vector<std::string> menu_names;
    for (const auto &toc_item: toc)
    {
        std::string indent(toc_item.indent_level * 2, ' ');
        // Leading indent + RTL visual reorder + right-align yields indent from the right edge
        menu_names.push_back(indent + toc_item.display_name);
    }

    auto current_toc_index = state.reader->get_toc_position(get_current_address(state)).toc_index;
    auto toc_select_menu = std::make_shared<SelectionMenu>(
        menu_names,
        state.sys_styling
    );
    toc_select_menu->set_rtl(toc_rtl);
    toc_select_menu->set_on_selection([&reader_view, last_toc_index=current_toc_index](uint32_t toc_index) {
        if (toc_index != last_toc_index)
        {
            reader_view.seek_to_toc_index(toc_index);
        }
    });
    toc_select_menu->set_close_on_select();

    // select current toc item
    if (current_toc_index < toc.size())
    {
        toc_select_menu->set_cursor_pos(current_toc_index);
    }

    toc_select_menu->set_default_on_keypress([](SDLKey key, SelectionMenu &toc) {
        if (key == SW_BTN_SELECT)
        {
            toc.close();
        }
    });

    state.view_stack.push(toc_select_menu);
}

void open_bookmarks_menu(ReaderView &reader_view, ReaderViewState &state)
{
    auto bookmarks = std::make_shared<std::vector<Bookmark>>(
        state.state_store.get_bookmarks(state.book_id)
    );
    if (bookmarks->empty())
    {
        state.view_stack.push(std::make_shared<PopupView>(
            "No bookmarks",
            SYSTEM_FONT,
            state.sys_styling
        ));
        return;
    }

    auto menu = std::make_shared<SelectionMenu>(
        bookmark_menu_names(*bookmarks),
        state.sys_styling
    );
    menu->set_rtl(bookmarks_rtl(state, *bookmarks));
    menu->set_close_on_select();

    // Jump nearest bookmark to current position
    {
        DocAddr current = get_current_address(state);
        uint32_t best = 0;
        for (uint32_t i = 0; i < bookmarks->size(); ++i)
        {
            if ((*bookmarks)[i].address <= current)
            {
                best = i;
            }
        }
        menu->set_cursor_pos(best);
    }

    menu->set_on_selection([&reader_view, bookmarks](uint32_t index) {
        if (index < bookmarks->size())
        {
            reader_view.seek_to_address((*bookmarks)[index].address);
        }
    });

    menu->set_default_on_keypress(
        [&state, bookmarks](SDLKey key, SelectionMenu &menu_ref) {
            if (key == SW_BTN_START)
            {
                menu_ref.close();
                return;
            }

            // Y deletes the focused bookmark
            if (key != SW_BTN_Y || bookmarks->empty())
            {
                return;
            }

            uint32_t index = menu_ref.get_cursor_pos();
            if (index >= bookmarks->size())
            {
                return;
            }

            bookmarks->erase(bookmarks->begin() + static_cast<std::ptrdiff_t>(index));
            state.state_store.set_bookmarks(state.book_id, *bookmarks);

            if (bookmarks->empty())
            {
                menu_ref.close();
                return;
            }

            menu_ref.set_entries(bookmark_menu_names(*bookmarks));
            menu_ref.set_cursor_pos(std::min(index, static_cast<uint32_t>(bookmarks->size()) - 1));
        }
    );

    state.view_stack.push(menu);
}

void flash_bookmark_toast(ReaderViewState &state, const char *message)
{
    constexpr uint32_t toast_ms = 900;
    state.view_stack.push(std::make_shared<PopupView>(
        message,
        SYSTEM_FONT,
        state.sys_styling,
        toast_ms
    ));
}

void toggle_bookmark_at_current(ReaderViewState &state)
{
    DocAddr address = get_current_address(state);
    auto bookmarks = state.state_store.get_bookmarks(state.book_id);

    auto it = std::find_if(bookmarks.begin(), bookmarks.end(), [address](const Bookmark &bm) {
        return bm.address == address;
    });

    if (it != bookmarks.end())
    {
        bookmarks.erase(it);
        state.state_store.set_bookmarks(state.book_id, std::move(bookmarks));
        flash_bookmark_toast(state, "Removed");
        return;
    }

    if (bookmarks.size() >= MAX_BOOKMARKS)
    {
        // Drop earliest in document order to make room
        bookmarks.erase(bookmarks.begin());
    }

    Bookmark bm;
    bm.address = address;
    bm.label = make_bookmark_label(state, address);

    auto insert_at = std::lower_bound(
        bookmarks.begin(),
        bookmarks.end(),
        bm,
        [](const Bookmark &a, const Bookmark &b) { return a.address < b.address; }
    );
    bookmarks.insert(insert_at, std::move(bm));
    state.state_store.set_bookmarks(state.book_id, std::move(bookmarks));
    flash_bookmark_toast(state, "Bookmarked");
}

} // namespace

ReaderView::ReaderView(
    std::filesystem::path path,
    std::shared_ptr<DocReader> reader,
    DocAddr seek_address,
    SystemStyling &sys_styling,
    TokenViewStyling &token_view_styling,
    ViewStack &view_stack,
    StateStore &state_store,
    std::string book_id
) : state(std::make_unique<ReaderViewState>(
        path,
        seek_address,
        reader,
        sys_styling,
        token_view_styling,
        token_view_styling.subscribe_to_changes([this]() {
            update_token_view_title(get_current_address(*state));
        }),
        view_stack,
        state_store,
        std::move(book_id)
    ))
{
    update_token_view_title(seek_address);

    // Update title info on scroll
    state->token_view->set_on_scroll([this](DocAddr address) {
        update_token_view_title(address);

        if (state->on_change_address)
        {
            state->on_change_address(address);
        }
    });
}

ReaderView::~ReaderView()
{
    state->token_view_styling.unsubscribe_from_changes(
        state->token_view_styling_sub_id
    );
    // Persist any position still waiting on the scroll throttle
    state->state_store.flush();
}

void ReaderView::update_token_view_title(DocAddr address)
{
    const auto &toc = state->reader->get_table_of_contents();
    auto toc_position = state->reader->get_toc_position(address);

    if (toc_position.toc_index < toc.size())
    {
        state->token_view->set_title(toc[toc_position.toc_index].display_name);
    }
    else
    {
        state->token_view->set_title(state->filename);
    }

    uint32_t progress_percent = (
        state->token_view_styling.get_progress_reporting() == ProgressReporting::CHAPTER_PERCENT ?
        toc_position.progress_percent :
        state->reader->get_global_progress_percent(address)
    );
    state->token_view->set_title_progress(progress_percent);
}

bool ReaderView::render(SDL_Surface *dest_surface, bool force_render)
{
    return state->token_view->render(dest_surface, force_render);
}

bool ReaderView::is_done()
{
    return state->is_done;
}

void ReaderView::on_keypress(SDLKey key)
{
    if (key == SW_BTN_B)
    {
        state->is_done = true;
        return;
    }

    switch (key) {
        case SW_BTN_A:
            state->token_view_styling.cycle_title_bar_progress();
            break;
        case SW_BTN_SELECT:
            open_toc_menu(*this, *state);
            break;
        case SW_BTN_START:
            open_bookmarks_menu(*this, *state);
            break;
        case SW_BTN_Y:
            toggle_bookmark_at_current(*state);
            break;
        case SW_BTN_L1:
        case SW_BTN_R1:
        case SW_BTN_L2:
        case SW_BTN_R2:
            {
                auto [chapter_l, chapter_r] = get_shoulder_keymap_chapter_lr(
                    state->sys_styling.get_shoulder_keymap()
                );
                if (key == chapter_l)
                {
                    seek_chapter_relative(-1);
                    break;
                }
                if (key == chapter_r)
                {
                    seek_chapter_relative(1);
                    break;
                }
            }
            // Page-turn shoulders
            state->token_view->on_keypress(key);
            break;
        default:
            state->token_view->on_keypress(key);
            break;
    }
}

void ReaderView::on_keyheld(SDLKey key, uint32_t hold_time_ms)
{
    state->token_view->on_keyheld(key, hold_time_ms);
}

void ReaderView::set_on_change_address(std::function<void(DocAddr)> callback)
{
    state->on_change_address = callback;
}

void ReaderView::seek_to_toc_index(uint32_t toc_index)
{
    auto address = state->reader->get_toc_item_address(toc_index);
    seek_to_address(address);
}

void ReaderView::seek_chapter_relative(int delta)
{
    const auto &toc = state->reader->get_table_of_contents();
    if (toc.empty() || delta == 0)
    {
        return;
    }

    int current = static_cast<int>(
        state->reader->get_toc_position(get_current_address(*state)).toc_index
    );
    int next = std::clamp(current + delta, 0, static_cast<int>(toc.size()) - 1);
    if (next != current)
    {
        seek_to_toc_index(static_cast<uint32_t>(next));
    }
}

void ReaderView::seek_to_address(DocAddr address)
{
    if (state->token_view)
    {
        state->token_view->seek_to_address(address);
        update_token_view_title(address);

        if (state->on_change_address)
        {
            state->on_change_address(address);
        }
    }
}
