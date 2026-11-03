#include "util/arabic_justify.h"
#include "util/text_visual.h"
#include "util/utf8_codec.h"

#include <gtest/gtest.h>

namespace {

// Approximate width: count codepoints after visual conversion (enough for unit tests).
int fake_measure(std::string_view logical)
{
    std::string visual = to_visual_line(logical, true);
    return static_cast<int>(utf8_to_codepoints(visual).size()) * 10;
}

bool contains_tatweel(const std::string &s)
{
    for (uint32_t cp : utf8_to_codepoints(s))
    {
        if (cp == 0x0640)
        {
            return true;
        }
    }
    return false;
}

} // namespace

TEST(ARABIC_JUSTIFY, no_op_without_arabic)
{
    std::string in = "Hello world";
    EXPECT_EQ(justify_arabic_line(in, 500, fake_measure), in);
}

TEST(ARABIC_JUSTIFY, no_op_when_already_wide_enough)
{
    // Dual-joining letters: بببب
    std::string in = "بببب";
    int natural = fake_measure(in);
    EXPECT_EQ(justify_arabic_line(in, natural, fake_measure), in);
}

TEST(ARABIC_JUSTIFY, inserts_tatweel_to_grow)
{
    std::string in = "بببب";
    int natural = fake_measure(in);
    std::string out = justify_arabic_line(in, natural + 40, fake_measure);
    EXPECT_NE(out, in);
    EXPECT_TRUE(contains_tatweel(out));
    EXPECT_GE(fake_measure(out), natural);
    EXPECT_LE(fake_measure(out), natural + 40);
}

TEST(ARABIC_JUSTIFY, empty_and_zero_target)
{
    EXPECT_EQ(justify_arabic_line("", 200, fake_measure), "");
    std::string in = "ببب";
    EXPECT_EQ(justify_arabic_line(in, 0, fake_measure), in);
}

TEST(ARABIC_JUSTIFY, space_fallback_when_no_join_sites)
{
    // Single Arabic letter + spaces + Latin — few/no kashida sites between dual joins
    std::string in = "ب  ب";
    int natural = fake_measure(in);
    std::string out = justify_arabic_line(in, natural + 30, fake_measure);
    // Should grow somehow (tatweel between بs if they join through spaces? spaces break join)
    // Spaces break joining, so expect extra spaces.
    EXPECT_GE(out.size(), in.size());
    EXPECT_GE(fake_measure(out), natural);
}
