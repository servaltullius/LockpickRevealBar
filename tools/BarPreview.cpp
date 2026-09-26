// Renders fully revealed bars for every palette, normal and inverted, into a PPM image
// using the same core code as the plugin. Usage: BarPreview <out.ppm> [center width partial]
#include "Core/Config.h"
#include "Core/Heat.h"
#include "Core/Palette.h"
#include "Core/Style.h"

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "usage: BarPreview <out.ppm> [center width partial]\n";
        return 1;
    }

    lrb::LockGeometry lock{ -45.0F, 10.33F, 9.25F };
    if (argc >= 5) {
        lock = { std::strtof(argv[2], nullptr), std::strtof(argv[3], nullptr), std::strtof(argv[4], nullptr) };
    }

    const lrb::Config config;
    constexpr int cells = 180;
    constexpr int cellW = 5;
    constexpr int barH = 24;
    constexpr int gap = 8;
    const int rows = lrb::PaletteCount() * 2;
    const int width = cells * cellW;
    const int height = rows * (barH + gap);

    std::vector<std::uint32_t> pixels(static_cast<std::size_t>(width * height), 0x101010);
    for (int row = 0; row < rows; ++row) {
        lrb::AttemptStyle style;
        style.palette = row / 2;
        style.inverted = (row % 2) == 1;
        style.noise = config.noise;
        style.seed = 1234;

        for (int i = 0; i < cells; ++i) {
            const float lo = lrb::kPickMin + lrb::kPickRange * static_cast<float>(i) / cells;
            const float hi = lrb::kPickMin + lrb::kPickRange * static_cast<float>(i + 1) / cells;
            const auto rgb = lrb::CellColor(config, style, lock, lo, hi, i, 1.0F);
            for (int y = 0; y < barH; ++y) {
                for (int x = 0; x < cellW; ++x) {
                    pixels[static_cast<std::size_t>((row * (barH + gap) + y) * width + i * cellW + x)] = rgb;
                }
            }
        }
    }

    std::ofstream out(argv[1], std::ios::binary);
    out << "P6\n" << width << ' ' << height << "\n255\n";
    for (const auto p : pixels) {
        const char rgb[3]{ static_cast<char>((p >> 16) & 0xFF), static_cast<char>((p >> 8) & 0xFF), static_cast<char>(p & 0xFF) };
        out.write(rgb, 3);
    }
    return out.good() ? 0 : 1;
}
