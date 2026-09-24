#include "Core/Style.h"

#include "Core/Palette.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace
{
    // Stable per-cell value in [-1, 1] so noise does not flicker between frames.
    [[nodiscard]] float CellNoise(std::uint32_t seed, int cell)
    {
        std::uint32_t x = seed ^ (static_cast<std::uint32_t>(cell) * 0x9E3779B9U);
        x ^= x >> 16;
        x *= 0x7FEB352DU;
        x ^= x >> 15;
        x *= 0x846CA68BU;
        x ^= x >> 16;
        return static_cast<float>(x) / static_cast<float>(0xFFFFFFFFU) * 2.0F - 1.0F;
    }
}

namespace lrb
{
    AttemptStyle RollStyle(const Config& config, std::uint32_t seed)
    {
        std::mt19937 rng(seed);
        std::uniform_real_distribution<float> unit(0.0F, 1.0F);

        AttemptStyle style;
        style.seed = rng();
        style.palette = config.randomPalette
                            ? std::uniform_int_distribution<int>(0, PaletteCount() - 1)(rng)
                            : config.fixedPalette % PaletteCount();
        style.inverted = unit(rng) < config.invertChance;
        if (config.bandsMax > 0 && unit(rng) < config.bandChance) {
            style.bands = std::uniform_int_distribution<int>(std::max(config.bandsMin, 2), std::max(config.bandsMax, 2))(rng);
        }
        style.noise = config.noise;
        return style;
    }

    float StyledHeat(const AttemptStyle& style, float heat, int cell)
    {
        if (heat < kSweetHeat) {
            if (style.bands > 0) {
                heat = std::floor(heat * static_cast<float>(style.bands)) / static_cast<float>(style.bands);
            }
            if (style.noise > 0.0F) {
                heat += style.noise * CellNoise(style.seed, cell);
            }
            // Keep a visible step between the hottest non-sweet cell and the sweet spot itself.
            heat = std::clamp(heat, 0.0F, 0.9F);
        }
        return style.inverted ? 1.0F - heat : heat;
    }

    std::uint32_t CellColor(
        const Config& config,
        const AttemptStyle& style,
        const LockGeometry& lock,
        float cellMin,
        float cellMax,
        int cell,
        float reveal)
    {
        if (reveal <= 0.0F) {
            return 0x000000;
        }

        std::uint32_t base = 0;
        if (config.sweetSpotMarker && CellContainsSweetSpot(lock, cellMin, cellMax)) {
            base = config.sweetSpotColor;
        } else {
            const float heat = CellHeat(lock, cellMin, cellMax, config.gamma);
            base = SamplePalette(style.palette, StyledHeat(style, heat, cell));
        }

        // Ease-in so a barely revealed cell is still clearly darker than a finished one.
        return ScaleColor(base, reveal * reveal * (3.0F - 2.0F * reveal));
    }
}
