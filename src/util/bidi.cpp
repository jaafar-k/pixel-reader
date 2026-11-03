#include "./bidi.h"

#include <algorithm>

namespace {

enum class DirClass {
    L,
    R,
    EN,  // European number
    AN,  // Arabic-Indic number
    N,   // Neutral
};

bool is_arabic_letter(uint32_t cp)
{
    return (cp >= 0x0600 && cp <= 0x06FF) ||
           (cp >= 0x0750 && cp <= 0x077F) ||
           (cp >= 0x08A0 && cp <= 0x08FF) ||
           (cp >= 0xFB50 && cp <= 0xFDFF) ||
           (cp >= 0xFE70 && cp <= 0xFEFF);
}

DirClass classify(uint32_t cp)
{
    if (cp >= '0' && cp <= '9')
    {
        return DirClass::EN;
    }
    if (cp >= 0x0660 && cp <= 0x0669) // Arabic-Indic digits
    {
        return DirClass::AN;
    }
    if (cp >= 0x06F0 && cp <= 0x06F9) // Extended Arabic-Indic digits
    {
        return DirClass::AN;
    }
    if ((cp >= 'A' && cp <= 'Z') || (cp >= 'a' && cp <= 'z') ||
        (cp >= 0x00C0 && cp <= 0x024F))
    {
        return DirClass::L;
    }
    if (is_arabic_letter(cp) || (cp >= 0x0590 && cp <= 0x05FF)) // Hebrew
    {
        return DirClass::R;
    }
    return DirClass::N;
}

} // namespace

void bidi_reorder_codepoints(std::vector<uint32_t> &cps, bool paragraph_rtl)
{
    if (cps.size() <= 1)
    {
        return;
    }

    const int n = static_cast<int>(cps.size());
    std::vector<DirClass> types(n);
    for (int i = 0; i < n; ++i)
    {
        types[i] = classify(cps[i]);
    }

    // Resolve neutrals from neighboring strong types (simplified W4/N1/N2).
    DirClass para = paragraph_rtl ? DirClass::R : DirClass::L;
    for (int i = 0; i < n; ++i)
    {
        if (types[i] != DirClass::N)
        {
            continue;
        }
        DirClass before = para;
        for (int j = i - 1; j >= 0; --j)
        {
            if (types[j] == DirClass::L || types[j] == DirClass::R)
            {
                before = types[j];
                break;
            }
            if (types[j] == DirClass::EN || types[j] == DirClass::AN)
            {
                before = DirClass::R; // numbers sit in RTL context often
                break;
            }
        }
        DirClass after = para;
        for (int j = i + 1; j < n; ++j)
        {
            if (types[j] == DirClass::L || types[j] == DirClass::R)
            {
                after = types[j];
                break;
            }
        }
        types[i] = (before == after) ? before : para;
    }

    // Weak: EN/AN become R in RTL paragraphs for mirroring purposes.
    for (int i = 0; i < n; ++i)
    {
        if (types[i] == DirClass::EN || types[i] == DirClass::AN)
        {
            types[i] = paragraph_rtl ? DirClass::R : DirClass::L;
        }
    }

    // Embedding levels: base +1 for opposite-direction runs.
    std::vector<int> levels(n, paragraph_rtl ? 1 : 0);
    for (int i = 0; i < n; ++i)
    {
        bool is_rtl = types[i] == DirClass::R;
        if (paragraph_rtl)
        {
            levels[i] = is_rtl ? 1 : 2;
        }
        else
        {
            levels[i] = is_rtl ? 1 : 0;
        }
    }

    // Reverse runs from highest level down to 1 (UAX#9 L2).
    int max_level = 0;
    for (int lvl : levels)
    {
        max_level = std::max(max_level, lvl);
    }
    for (int lvl = max_level; lvl >= 1; --lvl)
    {
        int i = 0;
        while (i < n)
        {
            if (levels[i] < lvl)
            {
                ++i;
                continue;
            }
            int j = i;
            while (j < n && levels[j] >= lvl)
            {
                ++j;
            }
            std::reverse(cps.begin() + i, cps.begin() + j);
            i = j;
        }
    }
}
