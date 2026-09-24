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
        test::Expect(lrb::StyledHeat(s, 0.89F, 0) < 1.0F, "non-sweet never reaches the sweet color");
        s.inverted = true;
        test::Near(lrb::StyledHeat(s, 1.0F, 0), 0.0F, 0.0F, "inverted sweet at the cold end");
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
        const auto full = lrb::CellColor(config, s, lock, -1.0F, 0.0F, 89, 1.0F);
        test::Expect((half & 0xFF) < (full & 0xFF), "partial reveal is darker");
        test::Expect(full == lrb::SamplePalette(5, 1.0F), "fully revealed sweet cell is the palette end");

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
    return test::Finish("StyleTests");
}
