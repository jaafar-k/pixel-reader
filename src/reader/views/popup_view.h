#ifndef POPUP_VIEW_H_
#define POPUP_VIEW_H_

#include "reader/view.h"
#include "util/timer.h"

#include <string>

struct SystemStyling;

class PopupView: public View
{
    bool _is_done = false;
    bool _needs_render = true;

    std::string message;
    std::string font_name;
    SystemStyling &styling;
    uint32_t styling_sub_id;
    uint32_t auto_dismiss_ms = 0;
    Timer life_timer;

public:
    // auto_dismiss_ms > 0 closes the popup without a keypress
    PopupView(
        const std::string &message,
        std::string font_name,
        SystemStyling &styling,
        uint32_t auto_dismiss_ms = 0
    );
    virtual ~PopupView();

    bool render(SDL_Surface *dest_surface, bool force_render) override;
    bool is_done() override;
    bool is_modal() override;
    void on_keypress(SDLKey key) override;
};

#endif
