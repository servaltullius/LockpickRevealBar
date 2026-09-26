#pragma once

#include <cstdint>
#include <filesystem>

namespace lrb
{
    enum class LayoutOverride
    {
        kAuto,
        kLegacy,
        kShifted
    };

    struct Config
    {
        // [General]
        bool enable{ true };
        bool debugLog{ false };
        LayoutOverride layout{ LayoutOverride::kAuto };

        // [Bar]
        int cells{ 180 };
        float widthPct{ 0.45F };
        float heightPct{ 0.022F };
        float topPct{ 0.07F };
        float alpha{ 92.0F };
        std::uint32_t borderColor{ 0x5A5A5A };
        bool showPickMarker{ true };
        std::uint32_t markerColor{ 0xFFFFFF };
        bool flipDirection{ false };

        // [Reveal]
        float radiusAtSkill0{ 2.5F };
        float radiusAtSkill100{ 14.0F };
        float speedAtSkill0{ 0.45F };
        float speedAtSkill100{ 2.5F };
        float skillCap{ 100.0F };
        float turningSpeedMult{ 2.0F };
        bool revealOnlyWhileTurning{ false };
        float fadePerSecond{ 0.0F };
        bool resetRevealOnPickBreak{ false };

        // [Heat]
        float gamma{ 1.6F };
        bool sweetSpotMarker{ false };
        std::uint32_t sweetSpotColor{ 0xFFFFFF };

        // [Obfuscation]
        bool rerollOnPickBreak{ true };
        float invertChance{ 0.5F };
        bool randomPalette{ true };
        int fixedPalette{ 0 };
        int bandsMin{ 5 };
        int bandsMax{ 10 };
        float bandChance{ 0.5F };
        float noise{ 0.03F };

        // [PickHealth]
        bool showPickHealth{ true };
        float healthHeightPct{ 0.007F };
        std::uint32_t healthColorHigh{ 0x5ED25E };
        std::uint32_t healthColorMid{ 0xE8C547 };
        std::uint32_t healthColorLow{ 0xE04B3A };

        [[nodiscard]] static Config Defaults();
    };

    [[nodiscard]] Config LoadConfig(const std::filesystem::path& path);
}
