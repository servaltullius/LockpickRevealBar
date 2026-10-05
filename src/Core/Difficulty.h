#pragma once

#include "Core/Config.h"

namespace lrb
{
    // Lock level as the game reports it: 0 novice .. 4 master.
    inline constexpr int kLockLevelCount = 5;
    inline constexpr int kNeutralLockLevel = 2;  // adept: plays exactly like the INI base values

    [[nodiscard]] const char* LockLevelName(int level);

    // The INI values describe an adept lock. Easier locks reveal wider and faster with fewer disguises;
    // harder locks shrink and slow the reveal, disguise more, and let revealed cells fog over again.
    // `difficultyStrength` scales the whole effect (0 = off, 2 = twice as steep).
    [[nodiscard]] Config ApplyDifficulty(const Config& base, int lockLevel);
}
