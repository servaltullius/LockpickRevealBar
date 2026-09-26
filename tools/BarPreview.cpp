// Renders fully revealed bars with the same core code as the plugin, as PPM images.
//   BarPreview <out.ppm>                                   every palette, normal and inverted rows
//   BarPreview <out.ppm> strip <palette> <inverted 0|1>    one framed bar
// Both forms accept an optional trailing [center width partial] (degrees).
#include "Core/Config.h"
#include "Core/Heat.h"
#include "Core/Palette.h"
#include "Core/Style.h"

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

namespace
{
    constexpr int kCells = 180;
    constexpr int kCellW = 5;
    constexpr int kBarH = 24;

    struct Image
    {
        int width;
        int height;
        std::vector<std::uint32_t> pixels;

        void Fill(int x0, int y0, int x1, int y1, std::uint32_t rgb)
        {
            for (int y = y0; y < y1; ++y) {
                for (int x = x0; x < x1; ++x) {
                    pixels[static_cast<std::size_t>(y * width + x)] = rgb;
                }
            }
        }

        bool Save(const char* path) const
        {
            std::ofstream out(path, std::ios::binary);
            out << "P6\n" << width << ' ' << height << "\n255\n";
            for (const auto p : pixels) {
                const char rgb[3]{ static_cast<char>((p >> 16) & 0xFF), static_cast<char>((p >> 8) & 0xFF), static_cast<char>(p & 0xFF) };
                out.write(rgb, 3);
            }
            return out.good();
        }
    };

    void DrawBar(Image& img, int x0, int y0, const lrb::Config& config, const lrb::AttemptStyle& style, const lrb::LockGeometry& lock)
    {
        for (int i = 0; i < kCells; ++i) {
            const float lo = lrb::kPickMin + lrb::kPickRange * static_cast<float>(i) / kCells;
            const float hi = lrb::kPickMin + lrb::kPickRange * static_cast<float>(i + 1) / kCells;
            const auto rgb = lrb::CellColor(config, style, lock, lo, hi, i, 1.0F);
            img.Fill(x0 + i * kCellW, y0, x0 + (i + 1) * kCellW, y0 + kBarH, rgb);
        }
    }

    [[nodiscard]] lrb::AttemptStyle MakeStyle(const lrb::Config& config, int palette, bool inverted)
    {
        lrb::AttemptStyle style;
        style.palette = palette;
        style.inverted = inverted;
        style.noise = config.noise;
        style.seed = 1234;
        return style;
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "usage: BarPreview <out.ppm> [strip <palette> <inverted>] [center width partial]\n";
        return 1;
    }

    const lrb::Config config;
    const bool strip = argc >= 5 && std::string_view(argv[2]) == "strip";
    const int geometryArg = strip ? 5 : 2;

    lrb::LockGeometry lock{ -45.0F, 10.33F, 9.25F };
    if (argc >= geometryArg + 3) {
        lock = { std::strtof(argv[geometryArg], nullptr), std::strtof(argv[geometryArg + 1], nullptr), std::strtof(argv[geometryArg + 2], nullptr) };
    }

    if (strip) {
        // 2px border like the in-game frame.
        constexpr int border = 2;
        Image img{ kCells * kCellW + border * 2, kBarH + border * 2, {} };
        img.pixels.assign(static_cast<std::size_t>(img.width * img.height), config.borderColor);
        DrawBar(img, border, border, config, MakeStyle(config, std::atoi(argv[3]), std::atoi(argv[4]) != 0), lock);
        return img.Save(argv[1]) ? 0 : 1;
    }

    constexpr int gap = 8;
    const int rows = lrb::PaletteCount() * 2;
    Image img{ kCells * kCellW, rows * (kBarH + gap), {} };
    img.pixels.assign(static_cast<std::size_t>(img.width * img.height), 0x101010);
    for (int row = 0; row < rows; ++row) {
        DrawBar(img, 0, row * (kBarH + gap), config, MakeStyle(config, row / 2, (row % 2) == 1), lock);
    }
    return img.Save(argv[1]) ? 0 : 1;
}
