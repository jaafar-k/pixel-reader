#ifndef BIDI_H_
#define BIDI_H_

#include <cstdint>
#include <vector>

// Minimal UAX#9-style reorder for Arabic + Latin mixed lines.
// Operates on already-shaped codepoints (presentation forms OK).
void bidi_reorder_codepoints(std::vector<uint32_t> &cps, bool paragraph_rtl);

#endif
