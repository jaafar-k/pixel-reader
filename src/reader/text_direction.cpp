#include "./text_direction.h"

#include "util/utf8_codec.h"

#include <stdexcept>

#define TEXT_DIR_AUTO "auto"
#define TEXT_DIR_LTR "ltr"
#define TEXT_DIR_RTL "rtl"

TextDirection get_next_text_direction(TextDirection direction)
{
    return static_cast<TextDirection>((static_cast<int>(direction) + 1) % 3);
}

const char *text_direction_display_name(TextDirection direction)
{
    switch (direction)
    {
        case TextDirection::Auto:
            return "Auto";
        case TextDirection::Ltr:
            return "LTR";
        case TextDirection::Rtl:
            return "RTL";
        default:
            throw std::runtime_error("Invalid text direction");
    }
}

std::optional<TextDirection> decode_text_direction(const std::string &value)
{
    if (value == TEXT_DIR_AUTO)
    {
        return TextDirection::Auto;
    }
    if (value == TEXT_DIR_LTR)
    {
        return TextDirection::Ltr;
    }
    if (value == TEXT_DIR_RTL)
    {
        return TextDirection::Rtl;
    }
    return {};
}

std::string encode_text_direction(TextDirection direction)
{
    switch (direction)
    {
        case TextDirection::Auto:
            return TEXT_DIR_AUTO;
        case TextDirection::Ltr:
            return TEXT_DIR_LTR;
        case TextDirection::Rtl:
            return TEXT_DIR_RTL;
        default:
            throw std::runtime_error("Invalid text direction");
    }
}

bool language_is_rtl(const std::string &language)
{
    if (language.size() < 2)
    {
        return false;
    }

    // Match ar, ara, ar-*, and other common RTL tags.
    auto starts_with_ci = [](const std::string &s, const char *prefix) {
        for (size_t i = 0; prefix[i]; ++i)
        {
            if (i >= s.size())
            {
                return false;
            }
            char c = s[i];
            if (c >= 'A' && c <= 'Z')
            {
                c = static_cast<char>(c - 'A' + 'a');
            }
            if (c != prefix[i])
            {
                return false;
            }
        }
        return true;
    };

    if (starts_with_ci(language, "ar") && (language.size() == 2 || language[2] == '-' || language[2] == '_'))
    {
        return true;
    }
    if (starts_with_ci(language, "ara") && (language.size() == 3 || language[3] == '-' || language[3] == '_'))
    {
        return true;
    }
    if (starts_with_ci(language, "he") && (language.size() == 2 || language[2] == '-' || language[2] == '_'))
    {
        return true;
    }
    if (starts_with_ci(language, "fa") && (language.size() == 2 || language[2] == '-' || language[2] == '_'))
    {
        return true;
    }
    if (starts_with_ci(language, "ur") && (language.size() == 2 || language[2] == '-' || language[2] == '_'))
    {
        return true;
    }
    return false;
}

bool text_contains_arabic(const std::string &text)
{
    const char *p = text.c_str();
    const char *end = p + text.size();
    while (p < end)
    {
        uint32_t cp = 0;
        const char *next = utf8_decode(p, end, &cp);
        if (!next)
        {
            break;
        }
        // Arabic, Arabic Supplement, Arabic Extended-A/B, Presentation Forms
        if ((cp >= 0x0600 && cp <= 0x06FF) ||
            (cp >= 0x0750 && cp <= 0x077F) ||
            (cp >= 0x08A0 && cp <= 0x08FF) ||
            (cp >= 0xFB50 && cp <= 0xFDFF) ||
            (cp >= 0xFE70 && cp <= 0xFEFF))
        {
            return true;
        }
        p = next;
    }
    return false;
}

bool resolve_effective_rtl(
    TextDirection direction,
    const std::string &language,
    const std::string &text_sample
)
{
    switch (direction)
    {
        case TextDirection::Ltr:
            return false;
        case TextDirection::Rtl:
            return true;
        case TextDirection::Auto:
        default:
            if (language_is_rtl(language))
            {
                return true;
            }
            return text_contains_arabic(text_sample);
    }
}
