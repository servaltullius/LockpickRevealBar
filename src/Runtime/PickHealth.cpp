#include "Runtime/PickHealth.h"

#include "Core/PickHealthScan.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <cmath>
#include <span>

namespace
{
    // AE Address Library ID of the static pick health float. Verified against 1.6.1170 and
    // 1.7.104 (same initial value 100.0 and identical neighbouring globals); present in every
    // AE database from 1.6.353 through 1.7.99.
    constexpr std::uint64_t kAEPickHealthID = 382911;

    const float* g_health = nullptr;

    [[nodiscard]] bool InRange(float value)
    {
        return std::isfinite(value) && value >= 0.0F && value <= 100.0F;
    }

    [[nodiscard]] const float* ScanTextSegment()
    {
        const auto text = REL::Module::get().segment(REL::Segment::textx);
        const auto* begin = text.pointer<const std::uint8_t>();
        const auto  offset = lrb::FindPickHealthGlobal({ begin, text.size() });
        return offset ? reinterpret_cast<const float*>(begin + *offset) : nullptr;
    }
}

namespace lrb
{
    void ResolvePickHealth()
    {
        // The code scan is checked on 1.6.1170 (agrees with the ID) and 1.7.104 (unique match), and
        // it is the only option on SE, which uses a different ID space. The AE ID is only a fallback,
        // so a future AE database without it is never queried while the scan works.
        const char* source = "code scan";
        g_health = ScanTextSegment();
        if (!g_health && REL::Module::IsAE()) {
            g_health = reinterpret_cast<const float*>(REL::ID(kAEPickHealthID).address());
            source = "address library";
        }

        if (g_health && !InRange(*g_health)) {
            SKSE::log::warn("Pick health candidate from {} holds {}; durability display disabled", source, *g_health);
            g_health = nullptr;
        }

        if (g_health) {
            SKSE::log::info("Pick health located via {} (current {:.1f})", source, *g_health);
        } else {
            SKSE::log::warn("Pick health not located; durability display disabled");
        }
    }

    std::optional<float> ReadPickHealth()
    {
        if (!g_health || !InRange(*g_health)) {
            return std::nullopt;
        }
        return *g_health / 100.0F;
    }
}
