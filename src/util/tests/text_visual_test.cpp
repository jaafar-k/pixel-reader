#include "reader/text_direction.h"
#include "util/text_visual.h"
#include "util/utf8_codec.h"

#include <gtest/gtest.h>

TEST(TEXT_DIRECTION, language_detection)
{
    EXPECT_TRUE(language_is_rtl("ar"));
    EXPECT_TRUE(language_is_rtl("ar-SA"));
    EXPECT_TRUE(language_is_rtl("ARA"));
    EXPECT_TRUE(language_is_rtl("he"));
    EXPECT_FALSE(language_is_rtl("en"));
    EXPECT_FALSE(language_is_rtl(""));
}

TEST(TEXT_DIRECTION, arabic_script_detection)
{
    EXPECT_TRUE(text_contains_arabic("مرحبا"));
    EXPECT_TRUE(text_contains_arabic("Hello مرحبا"));
    EXPECT_FALSE(text_contains_arabic("Hello"));
}

TEST(TEXT_DIRECTION, resolve_effective_rtl)
{
    EXPECT_FALSE(resolve_effective_rtl(TextDirection::Ltr, "ar", "مرحبا"));
    EXPECT_TRUE(resolve_effective_rtl(TextDirection::Rtl, "en", "Hello"));
    EXPECT_TRUE(resolve_effective_rtl(TextDirection::Auto, "ar", ""));
    EXPECT_TRUE(resolve_effective_rtl(TextDirection::Auto, "", "مرحبا بالعالم"));
    EXPECT_FALSE(resolve_effective_rtl(TextDirection::Auto, "en", "Hello world"));
}

TEST(TEXT_DIRECTION, encode_decode)
{
    EXPECT_EQ(decode_text_direction("auto"), TextDirection::Auto);
    EXPECT_EQ(decode_text_direction("ltr"), TextDirection::Ltr);
    EXPECT_EQ(decode_text_direction("rtl"), TextDirection::Rtl);
    EXPECT_EQ(encode_text_direction(TextDirection::Auto), "auto");
    EXPECT_EQ(get_next_text_direction(TextDirection::Auto), TextDirection::Ltr);
    EXPECT_EQ(get_next_text_direction(TextDirection::Rtl), TextDirection::Auto);
}

TEST(TEXT_VISUAL, shapes_arabic_letter_forms)
{
    // Beh (dual-joining) between joining letters should become medial form.
    // "ببب" -> initial, medial, final presentation forms
    std::string visual = to_visual_line("ببب", true);
    auto cps = utf8_to_codepoints(visual);
    ASSERT_EQ(cps.size(), 3u);
    // After bidi reverse for RTL paragraph, visual order is final, medial, initial
    EXPECT_EQ(cps[0], 0xFE90u); // beh final
    EXPECT_EQ(cps[1], 0xFE92u); // beh medial
    EXPECT_EQ(cps[2], 0xFE91u); // beh initial
}

TEST(TEXT_VISUAL, lam_alef_ligature)
{
    // لا (lam + alef)
    std::string visual = to_visual_line("لا", true);
    auto cps = utf8_to_codepoints(visual);
    ASSERT_EQ(cps.size(), 1u);
    EXPECT_EQ(cps[0], 0xFEFBu); // lam-alef isolated
}

TEST(TEXT_VISUAL, mixed_latin_arabic_keeps_latin_readable)
{
    std::string visual = to_visual_line("AB مرحبا", true);
    // Latin letters should still appear as A and B somewhere
    EXPECT_NE(visual.find('A'), std::string::npos);
    EXPECT_NE(visual.find('B'), std::string::npos);
}

TEST(TEXT_VISUAL, ltr_passthrough_latin)
{
    EXPECT_EQ(to_visual_line("Hello", false), "Hello");
}
