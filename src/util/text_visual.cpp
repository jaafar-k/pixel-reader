#include "./text_visual.h"

#include "./arabic_shape.h"
#include "./bidi.h"
#include "./utf8_codec.h"

#include <vector>

std::string to_visual_line(std::string_view logical, bool paragraph_rtl)
{
    if (logical.empty())
    {
        return {};
    }

    std::string as_str(logical);
    std::vector<uint32_t> cps = utf8_to_codepoints(as_str);
    arabic_shape_codepoints(cps);
    bidi_reorder_codepoints(cps, paragraph_rtl);
    return codepoints_to_utf8(cps);
}
