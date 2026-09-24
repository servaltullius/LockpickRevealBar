#pragma once

namespace lrb
{
    // Pick angle range used by the vanilla lockpicking menu (degrees).
    inline constexpr float kPickMin = -90.0F;
    inline constexpr float kPickMax = 90.0F;
    inline constexpr float kPickRange = kPickMax - kPickMin;

    // Heat bands. Gaps between bands make the zone borders readable once revealed.
    inline constexpr float kFarHigh = 0.55F;
    inline constexpr float kPartialLow = 0.60F;
    inline constexpr float kPartialHigh = 0.85F;
    inline constexpr float kSweetHeat = 1.0F;

    // Mirrors the game's own test: |pick - center| <= width / 2 opens the lock,
    // the next `partial` degrees on each side rotate it part way.
    struct LockGeometry
    {
        float center{ 0.0F };
        float width{ 0.0F };
        float partial{ 0.0F };
    };

    [[nodiscard]] float Heat(const LockGeometry& lock, float angle, float gamma);

    // Heat of the hottest point inside [lo, hi], so a sweet spot narrower than a cell is never skipped.
    [[nodiscard]] float CellHeat(const LockGeometry& lock, float lo, float hi, float gamma);

    [[nodiscard]] bool CellContainsSweetSpot(const LockGeometry& lock, float lo, float hi);
}
