#include "Core/Config.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>

namespace
{
    [[nodiscard]] std::string Trim(std::string value)
    {
        auto isSpace = [](unsigned char ch) { return std::isspace(ch) != 0; };
        value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), isSpace));
        value.erase(std::find_if_not(value.rbegin(), value.rend(), isSpace).base(), value.end());
        return value;
    }

    [[nodiscard]] std::string Lower(std::string value)
    {
        std::ranges::transform(value, value.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        return value;
    }

    // Strips a trailing ";" comment so "Cells=180 ; note" still parses. "#" is kept for #RRGGBB colors.
    [[nodiscard]] std::string StripInlineComment(const std::string& value)
    {
        const auto pos = value.find(';');
        return pos == std::string::npos ? value : Trim(value.substr(0, pos));
    }

    [[nodiscard]] std::optional<bool> ParseBool(std::string_view value)
    {
        const auto lowered = Lower(std::string(value));
        if (lowered == "true" || lowered == "1") {
            return true;
        }
        if (lowered == "false" || lowered == "0") {
            return false;
        }
        return std::nullopt;
    }

    void ParseBoolInto(const std::string& value, bool& target)
    {
        if (const auto parsed = ParseBool(value)) {
            target = *parsed;
        }
    }

    void ParseFloatInto(const std::string& value, float& target, float lo, float hi)
    {
        try {
            std::size_t parsedLength = 0;
            const auto parsed = std::stof(value, &parsedLength);
            if (parsedLength == value.size()) {
                target = std::clamp(parsed, lo, hi);
            }
        } catch (...) {
        }
    }

    void ParseIntInto(const std::string& value, int& target, int lo, int hi)
    {
        try {
            std::size_t parsedLength = 0;
            const auto parsed = std::stoi(value, &parsedLength);
            if (parsedLength == value.size()) {
                target = std::clamp(parsed, lo, hi);
            }
        } catch (...) {
        }
    }

    // Accepts 0xRRGGBB, #RRGGBB or plain hex RRGGBB.
    void ParseColorInto(std::string value, std::uint32_t& target)
    {
        if (value.starts_with('#')) {
            value.erase(0, 1);
        } else if (value.starts_with("0x") || value.starts_with("0X")) {
            value.erase(0, 2);
        }
        if (value.empty() || value.size() > 6) {
            return;
        }
        try {
            std::size_t parsedLength = 0;
            const auto parsed = std::stoul(value, &parsedLength, 16);
            if (parsedLength == value.size()) {
                target = static_cast<std::uint32_t>(parsed) & 0xFFFFFF;
            }
        } catch (...) {
        }
    }
}

namespace lrb
{
    Config Config::Defaults()
    {
        return {};
    }

    Config LoadConfig(const std::filesystem::path& path)
    {
        Config c = Config::Defaults();
        std::ifstream input(path);
        std::string line;

        while (std::getline(input, line)) {
            line = Trim(line);
            if (line.empty() || line.starts_with(';') || line.starts_with('#') || line.starts_with('[')) {
                continue;
            }

            const auto equals = line.find('=');
            if (equals == std::string::npos) {
                continue;
            }

            const auto key = Lower(Trim(line.substr(0, equals)));
            const auto value = StripInlineComment(Trim(line.substr(equals + 1)));

            if (key == "enable") {
                ParseBoolInto(value, c.enable);
            } else if (key == "debuglog") {
                ParseBoolInto(value, c.debugLog);
            } else if (key == "layout") {
                const auto v = Lower(value);
                if (v == "legacy") {
                    c.layout = LayoutOverride::kLegacy;
                } else if (v == "shifted") {
                    c.layout = LayoutOverride::kShifted;
                } else if (v == "auto") {
                    c.layout = LayoutOverride::kAuto;
                }
            } else if (key == "cells") {
                ParseIntInto(value, c.cells, 30, 360);
            } else if (key == "widthpct") {
                ParseFloatInto(value, c.widthPct, 0.05F, 1.0F);
            } else if (key == "heightpct") {
                ParseFloatInto(value, c.heightPct, 0.002F, 0.2F);
            } else if (key == "toppct") {
                ParseFloatInto(value, c.topPct, 0.0F, 0.98F);
            } else if (key == "alpha") {
                ParseFloatInto(value, c.alpha, 0.0F, 100.0F);
            } else if (key == "bordercolor") {
                ParseColorInto(value, c.borderColor);
            } else if (key == "showpickmarker") {
                ParseBoolInto(value, c.showPickMarker);
            } else if (key == "markercolor") {
                ParseColorInto(value, c.markerColor);
            } else if (key == "flipdirection") {
                ParseBoolInto(value, c.flipDirection);
            } else if (key == "radiusatskill0") {
                ParseFloatInto(value, c.radiusAtSkill0, 0.1F, 180.0F);
            } else if (key == "radiusatskill100") {
                ParseFloatInto(value, c.radiusAtSkill100, 0.1F, 180.0F);
            } else if (key == "speedatskill0") {
                ParseFloatInto(value, c.speedAtSkill0, 0.0F, 100.0F);
            } else if (key == "speedatskill100") {
                ParseFloatInto(value, c.speedAtSkill100, 0.0F, 100.0F);
            } else if (key == "skillcap") {
                ParseFloatInto(value, c.skillCap, 1.0F, 1000.0F);
            } else if (key == "turningspeedmult") {
                ParseFloatInto(value, c.turningSpeedMult, 0.0F, 100.0F);
            } else if (key == "revealonlywhileturning") {
                ParseBoolInto(value, c.revealOnlyWhileTurning);
            } else if (key == "fadepersecond") {
                ParseFloatInto(value, c.fadePerSecond, 0.0F, 100.0F);
            } else if (key == "resetrevealonpickbreak") {
                ParseBoolInto(value, c.resetRevealOnPickBreak);
            } else if (key == "gamma") {
                ParseFloatInto(value, c.gamma, 0.1F, 10.0F);
            } else if (key == "sweetspotmarker") {
                ParseBoolInto(value, c.sweetSpotMarker);
            } else if (key == "sweetspotcolor") {
                ParseColorInto(value, c.sweetSpotColor);
            } else if (key == "rerollonpickbreak") {
                ParseBoolInto(value, c.rerollOnPickBreak);
            } else if (key == "invertchance") {
                ParseFloatInto(value, c.invertChance, 0.0F, 1.0F);
            } else if (key == "randompalette") {
                ParseBoolInto(value, c.randomPalette);
            } else if (key == "fixedpalette") {
                ParseIntInto(value, c.fixedPalette, 0, 255);
            } else if (key == "bandsmin") {
                ParseIntInto(value, c.bandsMin, 0, 64);
            } else if (key == "bandsmax") {
                ParseIntInto(value, c.bandsMax, 0, 64);
            } else if (key == "bandchance") {
                ParseFloatInto(value, c.bandChance, 0.0F, 1.0F);
            } else if (key == "noise") {
                ParseFloatInto(value, c.noise, 0.0F, 0.5F);
            }
        }

        if (c.bandsMax < c.bandsMin) {
            c.bandsMax = c.bandsMin;
        }

        return c;
    }
}
