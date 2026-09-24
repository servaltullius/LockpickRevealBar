#include "Core/Heat.h"

#include <algorithm>
#include <cmath>

namespace lrb
{
    float Heat(const LockGeometry& lock, float angle, float gamma)
    {
        const float distance = std::fabs(angle - lock.center);
        const float half = std::max(lock.width, 0.0F) * 0.5F;
        const float partial = std::max(lock.partial, 0.0F);

        if (distance <= half) {
            return kSweetHeat;
        }

        if (partial > 0.0F && distance <= half + partial) {
            const float t = 1.0F - (distance - half) / partial;
            return kPartialLow + (kPartialHigh - kPartialLow) * t;
        }

        const float span = std::max(kPickRange - half - partial, 1.0F);
        const float x = std::clamp((distance - half - partial) / span, 0.0F, 1.0F);
        return kFarHigh * std::pow(1.0F - x, gamma);
    }

    float CellHeat(const LockGeometry& lock, float lo, float hi, float gamma)
    {
        return Heat(lock, std::clamp(lock.center, lo, hi), gamma);
    }

    bool CellContainsSweetSpot(const LockGeometry& lock, float lo, float hi)
    {
        const float half = std::max(lock.width, 0.0F) * 0.5F;
        return lock.center + half >= lo && lock.center - half <= hi;
    }
}
