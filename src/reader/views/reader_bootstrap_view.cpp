#include "./reader_bootstrap_view.h"

#include "./popup_view.h"
#include "./reader_view.h"
#include "filetypes/open_doc.h"
#include "reader/config.h"
#include "reader/ss_doc_reader_cache.h"
#include "reader/state_store.h"
#include "reader/system_styling.h"
#include "reader/view_stack.h"
#include "sys/screen.h"
#include "util/sdl_font_cache.h"
#include "util/sdl_utils.h"

#include <iostream>

struct ReaderBootstrapViewState
{
    std::filesystem::path book_path;
    SystemStyling &sys_styling;
    TokenViewStyling &token_view_styling;
    ViewStack &view_stack;
    StateStore &state_store;

    bool is_done = false;
    bool needs_render = true;

    ReaderBootstrapViewState(
        std::filesystem::path book_path,
        SystemStyling &sys_styling,
        TokenViewStyling &token_view_styling,
        ViewStack &view_stack,
        StateStore &state_store
    ) :
        book_path(book_path),
        sys_styling(sys_styling),
        token_view_styling(token_view_styling),
        view_stack(view_stack),
        state_store(state_store)
    {
    }
};

void ReaderBootstrapView::load_reader()
{
    state->is_done = true;

    auto &book_path = state->book_path;
    auto &sys_styling = state->sys_styling;
    auto &token_view_styling = state->token_view_styling;
    auto &view_stack = state->view_stack;
    auto &state_store = state->state_store;

    std::shared_ptr<DocReader> reader = create_doc_reader(book_path);
    SSDocReaderCache cache(state_store);
    if (!reader || !reader->open(cache))
    {
        std::cerr << "Failed to open " << book_path << std::endl;
        view_stack.push(std::make_shared<PopupView>("Error opening", SYSTEM_FONT, sys_styling));
        return;
    }

    state_store.set_current_book_path(book_path);

    auto book_id = reader->get_id();
    auto reader_view = std::make_shared<ReaderView>(
        book_path,
        reader,
        state_store.get_book_address(book_id).value_or(0),
        sys_styling,
        token_view_styling,
        view_stack,
        state_store,
        book_id
    );

    reader_view->set_on_change_address([&state_store, book_id](DocAddr addr) {
        state_store.set_book_address(book_id, addr);
    });

    view_stack.push(reader_view);
}

ReaderBootstrapView::ReaderBootstrapView(
    std::filesystem::path book_path,
    SystemStyling &sys_styling,
    TokenViewStyling &token_view_styling,
    ViewStack &view_stack,
    StateStore &state_store,
    std::function<void(std::function<void()>)> async
) : state(std::make_unique<ReaderBootstrapViewState>(book_path, sys_styling, token_view_styling, view_stack, state_store))
{
    // Perform asynchronously so that rendering can continue
    async([this](){ load_reader(); });
}

ReaderBootstrapView::~ReaderBootstrapView()
{
}

bool ReaderBootstrapView::render(SDL_Surface *dest_surface, bool force_render)
{
    bool perform_render = force_render || state->needs_render;
    if (perform_render)
    {
        const auto &theme = state->sys_styling.get_loaded_color_theme();

        SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        uint32_t surf_color = SDL_MapRGB(
            dest_surface->format,
            theme.background.r,
            theme.background.g,
            theme.background.b
        );
        SDL_FillRect(dest_surface, &rect, surf_color);

        TTF_Font *font = cached_load_font(SYSTEM_FONT, state->sys_styling.get_font_size());
        auto title = surface_unique_ptr { TTF_RenderUTF8_Shaded(
            font,
            "Opening...",
            theme.main_text,
            theme.background
        ) };
        if (title)
        {
            SDL_Rect title_rect = {
                static_cast<Sint16>(SCREEN_WIDTH / 2 - title->w / 2),
                static_cast<Sint16>(SCREEN_HEIGHT / 2 - title->h / 2),
                0,
                0
            };
            SDL_BlitSurface(title.get(), NULL, dest_surface, &title_rect);

            std::string name = state->book_path.filename().string();
            if (!name.empty())
            {
                auto subtitle = surface_unique_ptr { TTF_RenderUTF8_Shaded(
                    font,
                    name.c_str(),
                    theme.secondary_text,
                    theme.background
                ) };
                if (subtitle)
                {
                    SDL_Rect sub_rect = {
                        static_cast<Sint16>(SCREEN_WIDTH / 2 - subtitle->w / 2),
                        static_cast<Sint16>(title_rect.y + title->h + 8),
                        0,
                        0
                    };
                    SDL_BlitSurface(subtitle.get(), NULL, dest_surface, &sub_rect);
                }
            }
        }

        state->needs_render = false;
    }

    return perform_render;
}

bool ReaderBootstrapView::is_done()
{
    return state->is_done;
}

void ReaderBootstrapView::on_keypress(SDLKey)
{
}
