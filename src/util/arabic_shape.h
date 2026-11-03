#ifndef ARABIC_SHAPE_H_
#define ARABIC_SHAPE_H_

#include <cstdint>
#include <vector>

// Map Arabic letters in logical order to Unicode Presentation Forms.
void arabic_shape_codepoints(std::vector<uint32_t> &cps);

#endif
