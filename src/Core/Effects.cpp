#include "Core/Effects.h"

#include <algorithm>
#include <cmath>

namespace
{
    // Remaining-time fraction squared: fast fade at the start, gentle tail.
    [[nodiscard]] float EaseOut(float remaining, float total)
    {
        const float t = std::clamp(remaining / total, 0.0F, 1.0F);
        return t * t;
    }
}

namespace lrb
{
    void BarEffects::TriggerBreak(bool shake)
    {
        _break = kBreakFlashSeconds;
        if (shake) {
            _shake = kShakeSeconds;
        }
    }

    void BarEffects::Advance(float dt)
    {
        _sweet = std::max(0.0F, _sweet - dt);
        _break = std::max(0.0F, _break - dt);
        _shake = std::max(0.0F, _shake - dt);
    }

    float BarEffects::SweetFlashAlpha() const
    {
        return 90.0F * EaseOut(_sweet, kSweetFlashSeconds);
    }

    float BarEffects::BreakFlashAlpha() const
    {
        return 70.0F * EaseOut(_break, kBreakFlashSeconds);
    }

    float BarEffects::ShakeOffset() const
    {
        if (_shake <= 0.0F) {
            return 0.0F;
        }
        const float elapsed = kShakeSeconds - _shake;
        return std::sin(elapsed * 55.0F) * EaseOut(_shake, kShakeSeconds);
    }
}
