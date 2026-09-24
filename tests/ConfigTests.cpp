#include "Core/Config.h"
#include "TestUtil.h"

#include <filesystem>
#include <fstream>

namespace
{
    std::filesystem::path WriteTemp(const char* name, const char* contents)
    {
        const auto path = std::filesystem::temp_directory_path() / name;
        std::ofstream(path) << contents;
        return path;
    }
}

int main()
{
    {
        const auto c = lrb::LoadConfig(std::filesystem::temp_directory_path() / "lrb_missing_file.ini");
        test::Expect(c.enable && c.cells == 180, "missing file yields defaults");
    }
    {
        const auto path = WriteTemp("lrb_happy.ini",
            "[General]\n"
            "Enable=false\n"
            "Layout=Shifted\n"
            "[Bar]\n"
            "Cells=90 ; inline comment\n"
            "BorderColor=#112233\n"
            "MarkerColor=0xABCDEF\n"
            "FlipDirection=TRUE\n"
            "[Reveal]\n"
            "RadiusAtSkill100=20\n"
            "[Obfuscation]\n"
            "InvertChance=0.25\n"
            "BandsMin=8\n"
            "BandsMax=3\n");
        const auto c = lrb::LoadConfig(path);
        std::filesystem::remove(path);
        test::Expect(!c.enable, "Enable=false");
        test::Expect(c.layout == lrb::LayoutOverride::kShifted, "Layout is case-insensitive");
        test::Expect(c.cells == 90, "inline ; comment stripped");
        test::Expect(c.borderColor == 0x112233, "#RRGGBB color");
        test::Expect(c.markerColor == 0xABCDEF, "0xRRGGBB color");
        test::Expect(c.flipDirection, "TRUE parses");
        test::Near(c.radiusAtSkill100, 20.0F, 1e-6F, "float value");
        test::Near(c.invertChance, 0.25F, 1e-6F, "invert chance");
        test::Expect(c.bandsMax == 8, "BandsMax raised to BandsMin");
    }
    {
        const auto path = WriteTemp("lrb_bad.ini",
            "Cells=abc\n"
            "Alpha=500\n"
            "Noise=\n"
            "BorderColor=zzz\n"
            "Cells=5\n");
        const auto c = lrb::LoadConfig(path);
        std::filesystem::remove(path);
        test::Expect(c.cells == 30, "Cells clamped to minimum 30 after invalid entry");
        test::Near(c.alpha, 100.0F, 1e-6F, "Alpha clamped to 100");
        test::Near(c.noise, 0.05F, 1e-6F, "empty value keeps default");
        test::Expect(c.borderColor == 0x5A5A5A, "bad color keeps default");
    }
    return test::Finish("ConfigTests");
}
