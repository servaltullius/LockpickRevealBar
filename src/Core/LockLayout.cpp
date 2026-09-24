#include "Core/LockLayout.h"

#include "Core/Heat.h"

#include <cmath>

namespace lrb
{
    bool IsPlausible(const RawLockValues& v)
    {
        if (!std::isfinite(v.center) || !std::isfinite(v.width) || !std::isfinite(v.partial)) {
            return false;
        }
        // Tiny positive values are what an integer or handle looks like when read as a float.
        if (v.width < 0.01F || v.width > kPickRange) {
            return false;
        }
        if (v.partial != 0.0F && (v.partial < 0.01F || v.partial > kPickRange)) {
            return false;
        }
        // The game rolls the center inside [-90 + w/2, 90 - w/2].
        return std::fabs(v.center) <= kPickMax - v.width * 0.5F + 0.01F;
    }

    Layout ResolveLayout(LayoutOverride override, int major, int minor)
    {
        switch (override) {
        case LayoutOverride::kLegacy:
            return Layout::kLegacy;
        case LayoutOverride::kShifted:
            return Layout::kShifted;
        default:
            break;
        }
        if (major > 1 || (major == 1 && minor >= 7)) {
            return Layout::kShifted;
        }
        return Layout::kLegacy;
    }

    const char* LayoutName(Layout layout)
    {
        return layout == Layout::kShifted ? "shifted(1.7+)" : "legacy(SE/AE)";
    }
}
