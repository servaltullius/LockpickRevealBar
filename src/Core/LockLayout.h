#pragma once

#include "Core/Config.h"

namespace lrb
{
    // Where the sweet spot fields live inside LockpickingMenu.
    //  kLegacy : SE 1.5.x / AE 1.6.x (CommonLibSSE-NG layout; center is the unnamed float before sweetSpotAngle)
    //  kShifted: 1.7.x, where 0x14 bytes were inserted after pickBreakSeconds (verified against 1.7.104).
    enum class Layout
    {
        kLegacy,
        kShifted
    };

    struct RawLockValues
    {
        float center{ 0.0F };
        float width{ 0.0F };
        float partial{ 0.0F };
    };

    // False until the menu has rolled the lock, and for anything that is not a real lock setup.
    [[nodiscard]] bool IsPlausible(const RawLockValues& values);

    // No cross-layout guessing: before the lock is rolled the wrong layout can look valid.
    [[nodiscard]] Layout ResolveLayout(LayoutOverride override, int major, int minor);

    [[nodiscard]] const char* LayoutName(Layout layout);
}
