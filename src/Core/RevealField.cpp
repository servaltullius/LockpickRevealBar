#include "Core/RevealField.h"

#include "Core/Heat.h"

#include <algorithm>

namespace lrb
{
    RevealParams ComputeRevealParams(const Config& config, float lockpickingSkill, bool turning)
    {
        const float t = std::clamp(lockpickingSkill / config.skillCap, 0.0F, 1.0F);

        RevealParams params;
        params.radius = config.radiusAtSkill0 + (config.radiusAtSkill100 - config.radiusAtSkill0) * t;
        params.rate = config.speedAtSkill0 + (config.speedAtSkill100 - config.speedAtSkill0) * t;

        if (turning) {
            params.rate *= config.turningSpeedMult;
        } else if (config.revealOnlyWhileTurning) {
            params.rate = 0.0F;
        }
        return params;
    }

    void RevealField::Reset(int cells)
    {
        _values.assign(static_cast<std::size_t>(std::max(cells, 1)), 0.0F);
    }

    float RevealField::CellMin(int cell) const
    {
        return kPickMin + kPickRange * static_cast<float>(cell) / static_cast<float>(Cells());
    }

    float RevealField::CellMax(int cell) const
    {
        return kPickMin + kPickRange * static_cast<float>(cell + 1) / static_cast<float>(Cells());
    }

    bool RevealField::Step(float pickAngle, const RevealParams& params, float dt, float fadePerSecond)
    {
        bool changed = false;
        const float radius = std::max(params.radius, 0.001F);

        for (int i = 0; i < Cells(); ++i) {
            auto& value = _values[static_cast<std::size_t>(i)];
            const float before = value;

            // Distance from the pick to the nearest edge of the cell (0 when the pick is inside it).
            const float distance = std::max({ CellMin(i) - pickAngle, pickAngle - CellMax(i), 0.0F });

            if (distance < radius && params.rate > 0.0F) {
                const float weight = 1.0F - distance / radius;
                value = std::min(1.0F, value + params.rate * dt * weight);
            } else if (fadePerSecond > 0.0F) {
                value = std::max(0.0F, value - fadePerSecond * dt);
            }

            changed = changed || value != before;
        }
        return changed;
    }
}
