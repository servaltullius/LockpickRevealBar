#pragma once

#include <cstdint>

namespace lrb
{
    [[nodiscard]] int PaletteCount();

    [[nodiscard]] const char* PaletteName(int index);

    // t = 0 (cold) .. 1 (hot). Returns 0xRRGGBB.
    [[nodiscard]] std::uint32_t SamplePalette(int index, float t);

    // Multiplies every channel by `brightness` (0..1).
    [[nodiscard]] std::uint32_t ScaleColor(std::uint32_t rgb, float brightness);
}
