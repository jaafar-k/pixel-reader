#include "./arabic_justify.h"

#include "./arabic_shape.h"
#include "./utf8_codec.h"

#include <vector>

namespace {

constexpr uint32_t TATWEEL = 0x0640;
constexpr int MAX_TATWEEL_PER_SITE = 3;
constexpr int MAX_TOTAL_TATWEEL = 48;
constexpr int MAX_EXTRA_SPACES = 32;

bool is_arabic_letter_cp(uint32_t cp)
{
    return (cp >= 0x0600 && cp <= 0x06FF) ||
           (cp >= 0x0750 && cp <= 0x077F) ||
           (cp >= 0x08A0 && cp <= 0x08FF) ||
           (cp >= 0xFB50 && cp <= 0xFDFF) ||
           (cp >= 0xFE70 && cp <= 0xFEFF);
}

bool line_has_arabic(const std::vector<uint32_t> &cps)
{
    for (uint32_t cp : cps)
    {
        if (is_arabic_letter_cp(cp))
        {
            return true;
        }
    }
    return false;
}

// Returns insert indices (after a dual-joining letter's current tatweel run) where another tatweel may go.
std::vector<int> find_kashida_insert_points(const std::vector<uint32_t> &cps)
{
    std::vector<int> points;
    for (int i = 0; i < static_cast<int>(cps.size()); ++i)
    {
        uint32_t left = cps[i];
        if (left == TATWEEL || arabic_is_transparent(left))
        {
            continue;
        }
        if (!arabic_joins_left(arabic_join_type(left)))
        {
            continue;
        }

        // Skip existing tatweel run after this letter (and transparent marks).
        int j = i + 1;
        int tatweel_count = 0;
        while (j < static_cast<int>(cps.size()) &&
               (cps[j] == TATWEEL || arabic_is_transparent(cps[j])))
        {
            if (cps[j] == TATWEEL)
            {
                ++tatweel_count;
            }
            ++j;
        }
        if (tatweel_count >= MAX_TATWEEL_PER_SITE)
        {
            continue;
        }
        if (j >= static_cast<int>(cps.size()))
        {
            continue;
        }
        uint32_t right = cps[j];
        if (!arabic_joins_right(arabic_join_type(right)))
        {
            continue;
        }
        // Insert after the letter + existing tatweels (before marks attached to the next letter).
        int insert_at = i + 1 + tatweel_count;
        points.push_back(insert_at);
    }
    return points;
}

std::vector<int> find_space_insert_points(const std::vector<uint32_t> &cps)
{
    std::vector<int> points;
    for (int i = 0; i < static_cast<int>(cps.size()); ++i)
    {
        if (cps[i] == ' ')
        {
            points.push_back(i + 1);
        }
    }
    return points;
}

} // namespace

std::string justify_arabic_line(
    std::string_view logical,
    int target_width_px,
    const std::function<int(std::string_view)> &measure_visual_width
)
{
    if (logical.empty() || target_width_px <= 0 || !measure_visual_width)
    {
        return std::string(logical);
    }

    std::vector<uint32_t> cps = utf8_to_codepoints(std::string(logical));
    if (cps.empty() || !line_has_arabic(cps))
    {
        return std::string(logical);
    }

    int width = measure_visual_width(logical);
    if (width <= 0 || width >= target_width_px - 1)
    {
        return std::string(logical);
    }

    int total_tatweel = 0;
    int site_cursor = 0;
    while (width < target_width_px - 1 && total_tatweel < MAX_TOTAL_TATWEEL)
    {
        auto points = find_kashida_insert_points(cps);
        if (points.empty())
        {
            break;
        }

        int pi = site_cursor % static_cast<int>(points.size());
        int insert_at = points[pi];
        cps.insert(cps.begin() + insert_at, TATWEEL);
        ++total_tatweel;
        site_cursor = pi + 1;

        std::string candidate = codepoints_to_utf8(cps);
        int new_width = measure_visual_width(candidate);
        if (new_width <= 0 || new_width > target_width_px)
        {
            cps.erase(cps.begin() + insert_at);
            --total_tatweel;
            break;
        }
        width = new_width;
    }

    int extra_spaces = 0;
    int space_cursor = 0;
    while (width < target_width_px - 1 && extra_spaces < MAX_EXTRA_SPACES)
    {
        auto points = find_space_insert_points(cps);
        if (points.empty())
        {
            break;
        }
        int pi = space_cursor % static_cast<int>(points.size());
        int insert_at = points[pi];
        cps.insert(cps.begin() + insert_at, static_cast<uint32_t>(' '));
        ++extra_spaces;
        space_cursor = pi + 1;

        std::string candidate = codepoints_to_utf8(cps);
        int new_width = measure_visual_width(candidate);
        if (new_width <= 0 || new_width > target_width_px)
        {
            cps.erase(cps.begin() + insert_at);
            --extra_spaces;
            break;
        }
        width = new_width;
    }

    return codepoints_to_utf8(cps);
}
