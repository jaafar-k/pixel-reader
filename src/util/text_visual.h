#ifndef TEXT_VISUAL_H_
#define TEXT_VISUAL_H_

#include <string>
#include <string_view>

// Convert a logical UTF-8 line to visual order with Arabic joining.
std::string to_visual_line(std::string_view logical, bool paragraph_rtl);

#endif
