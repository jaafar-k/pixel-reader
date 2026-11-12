#include "./font_catalog.h"

#include "./config.h"
#include "sys/filesystem.h"
#include "util/sdl_font_cache.h"
#include "util/str_utils.h"

#include <algorithm>
#include <filesystem>
#include <iostream>

namespace
{

const std::vector<std::filesystem::path> EXTRA_FONTS = EXTRA_FONTS_LIST;

std::vector<std::string> available_fonts;

void discover_fonts()
{
    if (available_fonts.size())
    {
        return;
    }

    auto test_font = [](std::filesystem::path file_path) {
        auto norm_ext = to_lower(file_path.extension());
        return std::filesystem::exists(file_path) && (norm_ext == ".ttf" || norm_ext == ".ttc");
    };

    for (const auto &entry: directory_listing(FONT_DIR))
    {
        std::filesystem::path path = std::filesystem::path(FONT_DIR) / entry.name;
        if (!entry.is_dir && test_font(path))
        {
            available_fonts.push_back(path.string());
        }
    }

    for (const auto &path: EXTRA_FONTS)
    {
        if (test_font(path))
        {
            available_fonts.push_back(path.string());
        }
        else
        {
            std::cerr << "Skipping extra font: " << path.string() << std::endl;
        }
    }

    if (available_fonts.empty())
    {
        throw std::runtime_error("No fonts found");
    }
}

int get_font_index(const std::string &font_name)
{
    discover_fonts();

    const auto it = std::find(std::begin(available_fonts), std::end(available_fonts), font_name);
    uint32_t i = it - available_fonts.begin();
    if (i >= available_fonts.size())
    {
        return -1;
    }
    return i;
}

} // namespace

std::string get_valid_font_name(const std::string &preferred_font_name)
{
    int i = get_font_index(preferred_font_name);
    if (i < 0)
    {
        i = get_font_index(DEFAULT_FONT_NAME);
    }
    if (i < 0)
    {
        i = 0;
    }
    return available_fonts[i];
}

std::string get_prev_font_name(const std::string &font_name)
{
    int i = get_font_index(font_name);
    return available_fonts[(i + available_fonts.size() - 1) % available_fonts.size()];
}

std::string get_next_font_name(const std::string &font_name)
{
    int i = get_font_index(font_name);
    return available_fonts[(i + 1) % available_fonts.size()];
}

bool font_supports_arabic(TTF_Font *font)
{
    if (!font)
    {
        return false;
    }
    // Arabic letter Alef
    return TTF_GlyphIsProvided(font, 0x0627) != 0;
}

std::string get_content_font_name(const std::string &preferred_font_name, bool prefer_arabic)
{
    if (!prefer_arabic)
    {
        return get_valid_font_name(preferred_font_name);
    }

    // Always use the dedicated Arabic font for RTL/Arabic content.
    if (std::filesystem::exists(ARABIC_FONT_NAME))
    {
        return ARABIC_FONT_NAME;
    }

    // Fall back to any discovered font that has Arabic glyphs.
    discover_fonts();
    for (const auto &path : available_fonts)
    {
        TTF_Font *f = cached_load_font(path, DEFAULT_FONT_SIZE, FontLoadErrorOpt::NoThrow);
        if (f && font_supports_arabic(f))
        {
            return path;
        }
    }

    return get_valid_font_name(preferred_font_name);
}

