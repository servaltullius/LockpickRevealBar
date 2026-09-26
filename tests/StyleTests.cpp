#include "Core/Palette.h"
#include "Core/Style.h"
#include "TestUtil.h"

#include <set>

int main()
{
    lrb::Config config;

    {
        const auto a = lrb::RollStyle(config, 1234);
        const auto b = lrb::RollStyle(config, 1234);
        test::Expect(a.palette == b.palette && a.inverted == b.inverted && a.bands == b.bands && a.seed == b.seed,
            "same seed rolls the same style");
    }
    {
        std::set<int> palettes;
        int inverted = 0;
        int banded = 0;
        for (std::uint32_t seed = 0; seed < 400; ++seed) {
            const auto s = lrb::RollStyle(config, seed);
            palettes.insert(s.palette);
            inverted += s.inverted ? 1 : 0;
            banded += s.bands > 0 ? 1 : 0;
            test::Expect(s.bands == 0 || (s.bands >= config.bandsMin && s.bands <= config.bandsMax), "bands within range");
        }
        test::Expect(static_cast<int>(palettes.size()) == lrb::PaletteCount(), "every palette can be rolled");
        test::Expect(inverted > 120 && inverted < 280, "inversion near 50%");
        test::Expect(banded > 120 && banded < 280, "banding near 50%");
    }
    {
        lrb::Config fixed = config;
        fixed.randomPalette = false;
        fixed.fixedPalette = 3;
        fixed.invertChance = 0.0F;
        fixed.bandsMax = 0;
        const auto s = lrb::RollStyle(fixed, 99);
        test::Expect(s.palette == 3 && !s.inverted && s.bands == 0, "fixed style honours config");
    }
    {
        lrb::AttemptStyle s;
        s.noise = 0.0F;
        test::Near(lrb::StyledHeat(s, 1.0F, 0), 1.0F, 0.0F, "sweet stays at the hot end");
        test::Expect(lrb::StyledHeat(s, 0.99F, 0) <= lrb::kNonSweetCap, "non-sweet stays well below the sweet color");
        s.noise = 0.5F;
        for (int cell = 0; cell < 200; ++cell) {
            test::Expect(lrb::StyledHeat(s, lrb::kPartialHigh, cell) <= lrb::kNonSweetCap, "noise cannot push a cell near the sweet color");
        }
        s.noise = 0.0F;
        s.inverted = true;
        test::Near(lrb::StyledHeat(s, 1.0F, 0), 1.0F, 0.0F, "inversion never moves the sweet spot off the hot end");
        test::Near(lrb::StyledHeat(s, 0.0F, 0), lrb::kNonSweetCap, 1e-6F, "inverted: farthest cell is the brightest non-sweet");
        test::Near(lrb::StyledHeat(s, lrb::kPartialHigh, 0), lrb::kNonSweetCap - lrb::kPartialHigh, 1e-6F, "inverted: cells next to the sweet spot are dark");
        for (float h = 0.0F; h < 1.0F; h += 0.01F) {
            test::Expect(lrb::StyledHeat(s, h, 3) <= lrb::kNonSweetCap, "inverted non-sweet stays below the cap");
        }
        s.inverted = false;
        s.bands = 4;
        test::Near(lrb::StyledHeat(s, 0.49F, 0), 0.25F, 1e-6F, "banding floors to steps");
        s.bands = 0;
        s.noise = 0.2F;
        const float n1 = lrb::StyledHeat(s, 0.4F, 7);
        test::Near(lrb::StyledHeat(s, 0.4F, 7), n1, 0.0F, "noise is stable per cell");
    }
    {
        const lrb::LockGeometry lock{ 0.0F, 4.0F, 10.0F };
        lrb::AttemptStyle s;
        s.palette = 5;  // mono
        test::Expect(lrb::CellColor(config, s, lock, -1.0F, 0.0F, 89, 0.0F) == 0x000000, "unrevealed is black");
        const auto half = lrb::CellColor(config, s, lock, -1.0F, 0.0F, 89, 0.5F);
        const auto almost = lrb::CellColor(config, s, lock, -1.0F, 0.0F, 89, 0.99F);
        const auto full = lrb::CellColor(config, s, lock, -1.0F, 0.0F, 89, 1.0F);
        test::Expect(half == lrb::ScaleColor(lrb::kFogColor, 0.5F), "partial reveal is neutral fog, not a dimmed palette color");
        test::Expect(almost == lrb::ScaleColor(lrb::kFogColor, 0.99F), "palette color only appears once fully revealed");
        test::Expect(full == lrb::SamplePalette(5, 1.0F), "fully revealed sweet cell is the palette end");
        test::Expect((lrb::SamplePalette(5, 0.0F) & 0xFF) > (lrb::kFogColor & 0xFF), "coldest mono color is brighter than fog");

        lrb::Config marker = config;
        marker.sweetSpotMarker = true;
        marker.sweetSpotColor = 0x00FF00;
        test::Expect(lrb::CellColor(marker, s, lock, -1.0F, 0.0F, 89, 1.0F) == 0x00FF00, "sweet spot marker color");
    }
    {
        test::Expect(lrb::SamplePalette(0, 0.0F) == 0x2A3CFF, "palette start stop");
        test::Expect(lrb::SamplePalette(0, 1.0F) == 0xFF3A1A, "palette end stop");
        test::Expect(lrb::ScaleColor(0xFF8040, 0.5F) == 0x804020, "scale color halves channels");
    }
    {
        test::Expect(lrb::HealthColor(config, 1.0F) == config.healthColorHigh, "full health color");
        test::Expect(lrb::HealthColor(config, 0.5F) == config.healthColorMid, "half health color");
        test::Expect(lrb::HealthColor(config, 0.0F) == config.healthColorLow, "empty health color");
        test::Expect(lrb::HealthColor(config, 2.0F) == config.healthColorHigh, "health clamps above 1");
    }
    return test::Finish("StyleTests");
}
