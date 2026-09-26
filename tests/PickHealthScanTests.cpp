#include "Core/PickHealthScan.h"
#include "TestUtil.h"

#include <array>
#include <cstdint>
#include <vector>

namespace
{
    // SkyrimSE.exe 1.7.104, RVA 0x94F742..0x94F784: health += delta; store; clamp to 100; clamp to 0.
    // The health global is at RVA 0x20BAF54.
    constexpr std::array<std::uint8_t, 66> kAE17104{
        0xF3, 0x0F, 0x10, 0x0D, 0x0A, 0xB8, 0x76, 0x01, 0xF3, 0x0F, 0x58, 0xC8, 0xF3, 0x0F, 0x11, 0x0D,
        0xFE, 0xB7, 0x76, 0x01, 0xEB, 0x08, 0xF3, 0x0F, 0x10, 0x0D, 0xF4, 0xB7, 0x76, 0x01, 0x0F, 0x2F,
        0x0D, 0xAD, 0xC7, 0x20, 0x01, 0x76, 0x0C, 0xC7, 0x05, 0xE1, 0xB7, 0x76, 0x01, 0x00, 0x00, 0xC8,
        0x42, 0xEB, 0x0F, 0x0F, 0x2F, 0xCE, 0x73, 0x0A, 0xC7, 0x05, 0xD0, 0xB7, 0x76, 0x01, 0x00, 0x00,
        0x00, 0x00
    };
    constexpr std::ptrdiff_t kExpectedGlobal = 0x20BAF54 - 0x94F742;
}

int main()
{
    {
        const auto found = lrb::FindPickHealthGlobal(kAE17104);
        test::Expect(found.has_value(), "1.7.104 pick health site is found");
        test::Expect(found.value_or(0) == kExpectedGlobal, "RIP target resolves to the health global");
    }
    {
        // Same site with padding around it: offsets shift, the global stays the same absolute place.
        std::vector<std::uint8_t> padded(100, 0xCC);
        padded.insert(padded.end(), kAE17104.begin(), kAE17104.end());
        padded.insert(padded.end(), 100, 0xCC);
        const auto found = lrb::FindPickHealthGlobal(padded);
        test::Expect(found.value_or(0) == kExpectedGlobal + 100, "match is position independent");
    }
    {
        // Two copies of the site are ambiguous, so nothing is returned.
        std::vector<std::uint8_t> twice(kAE17104.begin(), kAE17104.end());
        twice.insert(twice.end(), kAE17104.begin(), kAE17104.end());
        test::Expect(!lrb::FindPickHealthGlobal(twice).has_value(), "ambiguous matches are rejected");
    }
    {
        // Without the clamp-to-zero store the 100.0 write alone is not enough.
        std::vector<std::uint8_t> noZero(kAE17104.begin(), kAE17104.begin() + 56);
        test::Expect(!lrb::FindPickHealthGlobal(noZero).has_value(), "requires the clamp-to-zero store");
    }
    {
        const std::array<std::uint8_t, 0> empty{};
        test::Expect(!lrb::FindPickHealthGlobal(empty).has_value(), "empty input");
    }
    return test::Finish("PickHealthScanTests");
}
