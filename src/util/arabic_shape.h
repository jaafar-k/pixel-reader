#ifndef ARABIC_SHAPE_H_
#define ARABIC_SHAPE_H_

#include <cstdint>
#include <vector>

enum class ArabicJoinType : uint8_t {
    None = 0,
    Right = 1,  // right-joining only (alef, dal, ...)
    Dual = 2,   // dual-joining
    Cause = 3,  // causes joining (tatweel)
};

ArabicJoinType arabic_join_type(uint32_t cp);
bool arabic_joins_left(ArabicJoinType t);
bool arabic_joins_right(ArabicJoinType t);
bool arabic_is_transparent(uint32_t cp);

// Map Arabic letters in logical order to Unicode Presentation Forms.
void arabic_shape_codepoints(std::vector<uint32_t> &cps);

#endif
