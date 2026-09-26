#pragma once

#include <cstdint>

namespace lrb
{
    [[nodiscard]] int PaletteCount();

    [[nodiscard]] const char* PaletteName(int index);

    // t = 0 (cold) .. 1 (hot). Returns 0xRRGGBB.
    [[nodiscard]] std::uint32_t SamplePalette(int index, float t);

    // Linear blend of two 0xRRGGBB colors, t = 0 -> a, t = 1 -> b.
    [[nodiscard]] std::uint32_t MixColors(std::uint32_t a, std::uint32_t b, float t);

    // Multiplies every channel by `brightness` (0..1).
    [[nodiscard]] std::uint32_t ScaleColor(std::uint32_t rgb, float brightness);
}
