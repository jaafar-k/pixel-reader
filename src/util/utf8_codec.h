#ifndef UTF8_CODEC_H_
#define UTF8_CODEC_H_

#include <cstdint>
#include <string>
#include <vector>

// Decode one UTF-8 codepoint. Returns next pointer, or nullptr on error.
inline const char *utf8_decode(const char *s, const char *end, uint32_t *out_cp)
{
    if (!s || s >= end)
    {
        return nullptr;
    }

    unsigned char c0 = static_cast<unsigned char>(*s);
    if (c0 < 0x80)
    {
        *out_cp = c0;
        return s + 1;
    }
    if ((c0 & 0xE0) == 0xC0)
    {
        if (s + 1 >= end)
        {
            return nullptr;
        }
        unsigned char c1 = static_cast<unsigned char>(s[1]);
        if ((c1 & 0xC0) != 0x80)
        {
            return nullptr;
        }
        *out_cp = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
        return s + 2;
    }
    if ((c0 & 0xF0) == 0xE0)
    {
        if (s + 2 >= end)
        {
            return nullptr;
        }
        unsigned char c1 = static_cast<unsigned char>(s[1]);
        unsigned char c2 = static_cast<unsigned char>(s[2]);
        if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80)
        {
            return nullptr;
        }
        *out_cp = ((c0 & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
        return s + 3;
    }
    if ((c0 & 0xF8) == 0xF0)
    {
        if (s + 3 >= end)
        {
            return nullptr;
        }
        unsigned char c1 = static_cast<unsigned char>(s[1]);
        unsigned char c2 = static_cast<unsigned char>(s[2]);
        unsigned char c3 = static_cast<unsigned char>(s[3]);
        if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80)
        {
            return nullptr;
        }
        *out_cp = ((c0 & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        return s + 4;
    }
    return nullptr;
}

inline void utf8_encode_append(std::string &out, uint32_t cp)
{
    if (cp < 0x80)
    {
        out.push_back(static_cast<char>(cp));
    }
    else if (cp < 0x800)
    {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    else if (cp < 0x10000)
    {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    else
    {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

inline std::vector<uint32_t> utf8_to_codepoints(const std::string &text)
{
    std::vector<uint32_t> cps;
    const char *p = text.c_str();
    const char *end = p + text.size();
    while (p < end)
    {
        uint32_t cp = 0;
        const char *next = utf8_decode(p, end, &cp);
        if (!next)
        {
            // Skip invalid byte
            ++p;
            continue;
        }
        cps.push_back(cp);
        p = next;
    }
    return cps;
}

inline std::string codepoints_to_utf8(const std::vector<uint32_t> &cps)
{
    std::string out;
    out.reserve(cps.size() * 2);
    for (uint32_t cp : cps)
    {
        utf8_encode_append(out, cp);
    }
    return out;
}

#endif
