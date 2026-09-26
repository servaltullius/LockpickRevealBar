#pragma once

#include <optional>

namespace lrb
{
    // Locates the game's current-lockpick health global. Call once at plugin load.
    void ResolvePickHealth();

    // Current lockpick health in [0, 1], or nothing if it could not be located.
    [[nodiscard]] std::optional<float> ReadPickHealth();
}
