#include "./arabic_shape.h"

namespace {

// Joining types
enum JoinType : uint8_t {
    J_NONE = 0,
    J_RIGHT = 1,  // right-joining only (alef, dal, ...)
    J_DUAL = 2,   // dual-joining
    J_CAUSE = 3,  // causes joining but is transparent for form (tatweel)
};

struct FormEntry {
    uint16_t isolated;
    uint16_t final_form;
    uint16_t initial;
    uint16_t medial;
    JoinType join;
};

// Sparse table covering common Arabic letters (U+0621–U+064A) + tatweel.
const FormEntry *lookup_form(uint32_t cp)
{
    switch (cp)
    {
        case 0x0621: { static const FormEntry e{0xFE80, 0xFE80, 0, 0, J_NONE}; return &e; } // hamza
        case 0x0622: { static const FormEntry e{0xFE81, 0xFE82, 0, 0, J_RIGHT}; return &e; } // alef madda
        case 0x0623: { static const FormEntry e{0xFE83, 0xFE84, 0, 0, J_RIGHT}; return &e; } // alef hamza above
        case 0x0624: { static const FormEntry e{0xFE85, 0xFE86, 0, 0, J_RIGHT}; return &e; } // waw hamza
        case 0x0625: { static const FormEntry e{0xFE87, 0xFE88, 0, 0, J_RIGHT}; return &e; } // alef hamza below
        case 0x0626: { static const FormEntry e{0xFE89, 0xFE8A, 0xFE8B, 0xFE8C, J_DUAL}; return &e; } // yeh hamza
        case 0x0627: { static const FormEntry e{0xFE8D, 0xFE8E, 0, 0, J_RIGHT}; return &e; } // alef
        case 0x0628: { static const FormEntry e{0xFE8F, 0xFE90, 0xFE91, 0xFE92, J_DUAL}; return &e; } // beh
        case 0x0629: { static const FormEntry e{0xFE93, 0xFE94, 0, 0, J_RIGHT}; return &e; } // teh marbuta
        case 0x062A: { static const FormEntry e{0xFE95, 0xFE96, 0xFE97, 0xFE98, J_DUAL}; return &e; } // teh
        case 0x062B: { static const FormEntry e{0xFE99, 0xFE9A, 0xFE9B, 0xFE9C, J_DUAL}; return &e; } // theh
        case 0x062C: { static const FormEntry e{0xFE9D, 0xFE9E, 0xFE9F, 0xFEA0, J_DUAL}; return &e; } // jeem
        case 0x062D: { static const FormEntry e{0xFEA1, 0xFEA2, 0xFEA3, 0xFEA4, J_DUAL}; return &e; } // hah
        case 0x062E: { static const FormEntry e{0xFEA5, 0xFEA6, 0xFEA7, 0xFEA8, J_DUAL}; return &e; } // khah
        case 0x062F: { static const FormEntry e{0xFEA9, 0xFEAA, 0, 0, J_RIGHT}; return &e; } // dal
        case 0x0630: { static const FormEntry e{0xFEAB, 0xFEAC, 0, 0, J_RIGHT}; return &e; } // thal
        case 0x0631: { static const FormEntry e{0xFEAD, 0xFEAE, 0, 0, J_RIGHT}; return &e; } // reh
        case 0x0632: { static const FormEntry e{0xFEAF, 0xFEB0, 0, 0, J_RIGHT}; return &e; } // zain
        case 0x0633: { static const FormEntry e{0xFEB1, 0xFEB2, 0xFEB3, 0xFEB4, J_DUAL}; return &e; } // seen
        case 0x0634: { static const FormEntry e{0xFEB5, 0xFEB6, 0xFEB7, 0xFEB8, J_DUAL}; return &e; } // sheen
        case 0x0635: { static const FormEntry e{0xFEB9, 0xFEBA, 0xFEBB, 0xFEBC, J_DUAL}; return &e; } // sad
        case 0x0636: { static const FormEntry e{0xFEBD, 0xFEBE, 0xFEBF, 0xFEC0, J_DUAL}; return &e; } // dad
        case 0x0637: { static const FormEntry e{0xFEC1, 0xFEC2, 0xFEC3, 0xFEC4, J_DUAL}; return &e; } // tah
        case 0x0638: { static const FormEntry e{0xFEC5, 0xFEC6, 0xFEC7, 0xFEC8, J_DUAL}; return &e; } // zah
        case 0x0639: { static const FormEntry e{0xFEC9, 0xFECA, 0xFECB, 0xFECC, J_DUAL}; return &e; } // ain
        case 0x063A: { static const FormEntry e{0xFECD, 0xFECE, 0xFECF, 0xFED0, J_DUAL}; return &e; } // ghain
        case 0x0640: { static const FormEntry e{0x0640, 0x0640, 0x0640, 0x0640, J_CAUSE}; return &e; } // tatweel
        case 0x0641: { static const FormEntry e{0xFED1, 0xFED2, 0xFED3, 0xFED4, J_DUAL}; return &e; } // feh
        case 0x0642: { static const FormEntry e{0xFED5, 0xFED6, 0xFED7, 0xFED8, J_DUAL}; return &e; } // qaf
        case 0x0643: { static const FormEntry e{0xFED9, 0xFEDA, 0xFEDB, 0xFEDC, J_DUAL}; return &e; } // kaf
        case 0x0644: { static const FormEntry e{0xFEDD, 0xFEDE, 0xFEDF, 0xFEE0, J_DUAL}; return &e; } // lam
        case 0x0645: { static const FormEntry e{0xFEE1, 0xFEE2, 0xFEE3, 0xFEE4, J_DUAL}; return &e; } // meem
        case 0x0646: { static const FormEntry e{0xFEE5, 0xFEE6, 0xFEE7, 0xFEE8, J_DUAL}; return &e; } // noon
        case 0x0647: { static const FormEntry e{0xFEE9, 0xFEEA, 0xFEEB, 0xFEEC, J_DUAL}; return &e; } // heh
        case 0x0648: { static const FormEntry e{0xFEED, 0xFEEE, 0, 0, J_RIGHT}; return &e; } // waw
        case 0x0649: { static const FormEntry e{0xFEEF, 0xFEF0, 0, 0, J_RIGHT}; return &e; } // alef maksura
        case 0x064A: { static const FormEntry e{0xFEF1, 0xFEF2, 0xFEF3, 0xFEF4, J_DUAL}; return &e; } // yeh
        default:
            return nullptr;
    }
}

bool is_transparent(uint32_t cp)
{
    // Arabic combining marks / harakat
    return (cp >= 0x064B && cp <= 0x065F) || cp == 0x0670 ||
           (cp >= 0x06D6 && cp <= 0x06ED) || (cp >= 0x08D3 && cp <= 0x08FF);
}

JoinType join_type(uint32_t cp)
{
    if (const FormEntry *e = lookup_form(cp))
    {
        return e->join;
    }
    return J_NONE;
}

bool joins_left(JoinType t)
{
    return t == J_DUAL || t == J_CAUSE;
}

bool joins_right(JoinType t)
{
    return t == J_RIGHT || t == J_DUAL || t == J_CAUSE;
}

// Lam + Alef ligatures (isolated/final pairs)
bool try_lam_alef(uint32_t alef, bool joins_prev, uint32_t *out_lig)
{
    uint32_t isolated = 0;
    uint32_t final_form = 0;
    switch (alef)
    {
        case 0x0622: isolated = 0xFEF5; final_form = 0xFEF6; break; // lam+alef madda
        case 0x0623: isolated = 0xFEF7; final_form = 0xFEF8; break; // lam+alef hamza above
        case 0x0625: isolated = 0xFEF9; final_form = 0xFEFA; break; // lam+alef hamza below
        case 0x0627: isolated = 0xFEFB; final_form = 0xFEFC; break; // lam+alef
        default:
            return false;
    }
    *out_lig = joins_prev ? final_form : isolated;
    return true;
}

int next_letter_index(const std::vector<uint32_t> &cps, int from)
{
    for (int i = from; i < static_cast<int>(cps.size()); ++i)
    {
        if (is_transparent(cps[i]))
        {
            continue;
        }
        return i;
    }
    return -1;
}

int prev_letter_index(const std::vector<uint32_t> &cps, int from)
{
    for (int i = from; i >= 0; --i)
    {
        if (is_transparent(cps[i]))
        {
            continue;
        }
        return i;
    }
    return -1;
}

} // namespace

void arabic_shape_codepoints(std::vector<uint32_t> &cps)
{
    if (cps.empty())
    {
        return;
    }

    std::vector<uint32_t> out;
    out.reserve(cps.size());

    for (int i = 0; i < static_cast<int>(cps.size()); ++i)
    {
        uint32_t cp = cps[i];
        if (is_transparent(cp))
        {
            out.push_back(cp);
            continue;
        }

        const FormEntry *form = lookup_form(cp);
        if (!form)
        {
            out.push_back(cp);
            continue;
        }

        // Lam-Alef ligature
        if (cp == 0x0644)
        {
            int ni = next_letter_index(cps, i + 1);
            if (ni >= 0)
            {
                uint32_t lig = 0;
                int pi = prev_letter_index(cps, i - 1);
                bool join_prev = pi >= 0 && joins_left(join_type(cps[pi]));
                if (try_lam_alef(cps[ni], join_prev, &lig))
                {
                    out.push_back(lig);
                    // Copy transparent marks between lam and alef, skip alef
                    for (int j = i + 1; j < ni; ++j)
                    {
                        out.push_back(cps[j]);
                    }
                    i = ni;
                    continue;
                }
            }
        }

        int pi = prev_letter_index(cps, i - 1);
        int ni = next_letter_index(cps, i + 1);
        bool join_prev = pi >= 0 && joins_left(join_type(cps[pi]));
        bool join_next = ni >= 0 && joins_right(join_type(cps[ni])) && joins_left(form->join);

        uint32_t shaped = form->isolated;
        if (join_prev && join_next && form->medial)
        {
            shaped = form->medial;
        }
        else if (join_prev && form->final_form)
        {
            shaped = form->final_form;
        }
        else if (join_next && form->initial)
        {
            shaped = form->initial;
        }
        else if (form->isolated)
        {
            shaped = form->isolated;
        }

        out.push_back(shaped);
    }

    cps.swap(out);
}
