#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace lrb
{
    // The current lockpick's health (0..100) is a static float, not a LockpickingMenu member.
    // The pick-damage routine ends with:
    //   movss [health], xmmN            ; health += -100 / breakSeconds * dt
    //   ...
    //   mov   dword ptr [health], 100.0 ; clamp high
    //   ...
    //   mov   dword ptr [health], 0     ; clamp low
    // Returns the global's position relative to code.data() (RIP targets may fall outside the span),
    // or nothing unless exactly one site matches.
    [[nodiscard]] std::optional<std::ptrdiff_t> FindPickHealthGlobal(std::span<const std::uint8_t> code);
}
