#include "Core/Difficulty.h"

#include <algorithm>
#include <array>

namespace
{
    // Index = lock level (novice .. master); index 2 is neutral.
    constexpr std::array<float, lrb::kLockLevelCount> kRevealMult{ 1.35F, 1.15F, 1.0F, 0.85F, 0.7F };
    constexpr std::array<float, lrb::kLockLevelCount> kDisguiseMult{ 0.3F, 0.6F, 1.0F, 1.4F, 1.7F };
    constexpr std::array<float, lrb::kLockLevelCount> kNoiseMult{ 0.5F, 0.75F, 1.0F, 1.3F, 1.6F };
    constexpr std::array<float, lrb::kLockLevelCount> kFadeAdd{ 0.0F, 0.0F, 0.0F, 0.04F, 0.1F };

    constexpr std::array<const char*, lrb::kLockLevelCount> kNames{ "novice", "apprentice", "adept", "expert", "master" };
}

namespace lrb
{
    const char* LockLevelName(int level)
    {
        return level >= 0 && level < kLockLevelCount ? kNames[static_cast<std::size_t>(level)] : "unknown";
    }

    Config ApplyDifficulty(const Config& base, int lockLevel)
    {
        if (!base.difficultyScaling || lockLevel < 0 || lockLevel >= kLockLevelCount) {
            return base;
        }

        const auto i = static_cast<std::size_t>(lockLevel);
        const float s = base.difficultyStrength;
        auto scaled = [s](float mult) { return std::max(0.05F, 1.0F + (mult - 1.0F) * s); };

        Config c = base;
        c.radiusAtSkill0 *= scaled(kRevealMult[i]);
        c.radiusAtSkill100 *= scaled(kRevealMult[i]);
        c.speedAtSkill0 *= scaled(kRevealMult[i]);
        c.speedAtSkill100 *= scaled(kRevealMult[i]);
        c.invertChance = std::clamp(base.invertChance * scaled(kDisguiseMult[i]), 0.0F, 1.0F);
        c.bandChance = std::clamp(base.bandChance * scaled(kDisguiseMult[i]), 0.0F, 1.0F);
        c.noise = std::clamp(base.noise * scaled(kNoiseMult[i]), 0.0F, 0.5F);
        c.fadePerSecond = base.fadePerSecond + kFadeAdd[i] * s;
        return c;
    }
}
