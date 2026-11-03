#ifndef TEXT_DIRECTION_H_
#define TEXT_DIRECTION_H_

#include <optional>
#include <string>

enum class TextDirection {
    Auto,
    Ltr,
    Rtl
};

TextDirection get_next_text_direction(TextDirection direction);
const char *text_direction_display_name(TextDirection direction);

std::optional<TextDirection> decode_text_direction(const std::string &value);
std::string encode_text_direction(TextDirection direction);

bool language_is_rtl(const std::string &language);
bool text_contains_arabic(const std::string &text);

// Resolve Auto using OPF language and/or text sample.
bool resolve_effective_rtl(
    TextDirection direction,
    const std::string &language,
    const std::string &text_sample
);

#endif
