#pragma once

#include "Core/Config.h"
#include "Core/Heat.h"

#include <cstdint>

namespace lrb
{
    // Per-attempt disguise so a few revealed cells do not give away which way the sweet spot lies.
    struct AttemptStyle
    {
        int palette{ 0 };
        bool inverted{ false };  // sweet spot shows as the cold end of the palette
        int bands{ 0 };          // 0 = smooth gradient
        float noise{ 0.0F };
        std::uint32_t seed{ 0 };
    };

    // Cells still being uncovered are drawn as neutral fog, never as a dimmed palette color,
    // so a half-revealed cell cannot be mistaken for a colder one.
    inline constexpr std::uint32_t kFogColor = 0x262626;

    [[nodiscard]] AttemptStyle RollStyle(const Config& config, std::uint32_t seed);

    // Heat after banding, noise and inversion. Sweet cells stay at the exact palette end.
    [[nodiscard]] float StyledHeat(const AttemptStyle& style, float heat, int cell);

    // Final cell color: black when unrevealed, brightening fog while being uncovered, the styled palette color once fully revealed.
    [[nodiscard]] std::uint32_t CellColor(
        const Config& config,
        const AttemptStyle& style,
        const LockGeometry& lock,
        float cellMin,
        float cellMax,
        int cell,
        float reveal);

    // Lockpick durability bar color for health in [0, 1]: low -> mid -> high.
    [[nodiscard]] std::uint32_t HealthColor(const Config& config, float health);
}
