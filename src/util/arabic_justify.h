#ifndef ARABIC_JUSTIFY_H_
#define ARABIC_JUSTIFY_H_

#include <functional>
#include <string>
#include <string_view>

// Stretch a logical Arabic line toward target_width_px using tatweel (kashida),
// then extra spaces if needed. measure_visual_width must size the shaped visual form.
std::string justify_arabic_line(
    std::string_view logical,
    int target_width_px,
    const std::function<int(std::string_view)> &measure_visual_width
);

#endif
