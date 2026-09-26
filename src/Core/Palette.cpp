#include "Core/Palette.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <span>

namespace
{
    struct Palette
    {
        const char* name;
        std::span<const std::uint32_t> stops;
    };

    // Both ends stay visibly brighter than black so a revealed cold cell still reads as revealed.
    constexpr std::array<std::uint32_t, 5> kThermal{ 0x2A3CFF, 0x00B8FF, 0x2EE86A, 0xFFE040, 0xFF3A1A };
    constexpr std::array<std::uint32_t, 4> kEmber{ 0x5A2A18, 0xA8401A, 0xFF8A1E, 0xFFF0B0 };
    constexpr std::array<std::uint32_t, 4> kToxic{ 0x1E5A2A, 0x2FA04A, 0xB4F040, 0xF4FFE0 };
    constexpr std::array<std::uint32_t, 4> kArcane{ 0x4A2A7A, 0x8A3CD0, 0xE070FF, 0xFFF0FF };
    constexpr std::array<std::uint32_t, 4> kFrost{ 0x1A3A6A, 0x1878C0, 0x50D8FF, 0xF0FFFF };
    constexpr std::array<std::uint32_t, 2> kMono{ 0x5A5A5A, 0xF4F4F4 };
    constexpr std::array<std::uint32_t, 3> kBlood{ 0x3A3A48, 0x9A2030, 0xFF6060 };

    constexpr std::array<Palette, 7> kPalettes{ {
        { "thermal", kThermal },
        { "ember", kEmber },
        { "toxic", kToxic },
        { "arcane", kArcane },
        { "frost", kFrost },
        { "mono", kMono },
        { "blood", kBlood },
    } };

    [[nodiscard]] std::uint32_t Mix(std::uint32_t a, std::uint32_t b, float t)
    {
        auto channel = [t](std::uint32_t x, std::uint32_t y, int shift) {
            const float cx = static_cast<float>((x >> shift) & 0xFF);
            const float cy = static_cast<float>((y >> shift) & 0xFF);
            const auto value = static_cast<std::uint32_t>(std::lround(cx + (cy - cx) * t));
            return std::min<std::uint32_t>(value, 0xFF) << shift;
        };
        return channel(a, b, 16) | channel(a, b, 8) | channel(a, b, 0);
    }

    [[nodiscard]] const Palette& Get(int index)
    {
        const int count = static_cast<int>(kPalettes.size());
        return kPalettes[static_cast<std::size_t>(((index % count) + count) % count)];
    }
}

namespace lrb
{
    int PaletteCount()
    {
        return static_cast<int>(kPalettes.size());
    }

    const char* PaletteName(int index)
    {
        return Get(index).name;
    }

    std::uint32_t SamplePalette(int index, float t)
    {
        const auto& stops = Get(index).stops;
        const float scaled = std::clamp(t, 0.0F, 1.0F) * static_cast<float>(stops.size() - 1);
        const auto lo = std::min(static_cast<std::size_t>(scaled), stops.size() - 2);
        return Mix(stops[lo], stops[lo + 1], scaled - static_cast<float>(lo));
    }

    std::uint32_t MixColors(std::uint32_t a, std::uint32_t b, float t)
    {
        return Mix(a, b, std::clamp(t, 0.0F, 1.0F));
    }

    std::uint32_t ScaleColor(std::uint32_t rgb, float brightness)
    {
        return Mix(0x000000, rgb, std::clamp(brightness, 0.0F, 1.0F));
    }
}
