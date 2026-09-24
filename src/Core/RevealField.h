#pragma once

#include "Core/Config.h"

#include <vector>

namespace lrb
{
    struct RevealParams
    {
        float radius{ 0.0F };  // degrees around the pick that get revealed
        float rate{ 0.0F };    // reveal gained per second at the pick itself
    };

    [[nodiscard]] RevealParams ComputeRevealParams(const Config& config, float lockpickingSkill, bool turning);

    // How much of each bar cell the player has uncovered (0 = black, 1 = fully shown).
    class RevealField
    {
    public:
        void Reset(int cells);

        [[nodiscard]] int Cells() const noexcept { return static_cast<int>(_values.size()); }
        [[nodiscard]] float Get(int cell) const { return _values[static_cast<std::size_t>(cell)]; }

        // Angle interval covered by a cell, in pick degrees.
        [[nodiscard]] float CellMin(int cell) const;
        [[nodiscard]] float CellMax(int cell) const;

        // Returns true when any cell value changed.
        bool Step(float pickAngle, const RevealParams& params, float dt, float fadePerSecond);

    private:
        std::vector<float> _values;
    };
}
