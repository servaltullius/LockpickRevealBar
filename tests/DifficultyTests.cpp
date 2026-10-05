#include "Core/Difficulty.h"
#include "TestUtil.h"

#include <string>

int main()
{
    lrb::Config base;

    {
        const auto adept = lrb::ApplyDifficulty(base, 2);
        test::Near(adept.radiusAtSkill100, base.radiusAtSkill100, 1e-6F, "adept keeps the base radius");
        test::Near(adept.invertChance, base.invertChance, 1e-6F, "adept keeps the base inversion chance");
        test::Near(adept.fadePerSecond, base.fadePerSecond, 1e-6F, "adept adds no fog");
    }
    {
        const auto novice = lrb::ApplyDifficulty(base, 0);
        const auto master = lrb::ApplyDifficulty(base, 4);
        test::Expect(novice.radiusAtSkill0 > base.radiusAtSkill0 && master.radiusAtSkill0 < base.radiusAtSkill0, "radius shrinks with difficulty");
        test::Expect(novice.speedAtSkill100 > master.speedAtSkill100, "speed shrinks with difficulty");
        test::Expect(novice.invertChance < base.invertChance && master.invertChance > base.invertChance, "inversion grows with difficulty");
        test::Expect(master.invertChance <= 1.0F && master.bandChance <= 1.0F, "chances stay probabilities");
        test::Expect(master.noise > novice.noise && master.noise <= 0.5F, "noise grows and stays clamped");
        test::Expect(master.fadePerSecond > 0.0F && novice.fadePerSecond == base.fadePerSecond, "only hard locks fog over");
    }
    {
        lrb::Config off = base;
        off.difficultyScaling = false;
        test::Near(lrb::ApplyDifficulty(off, 4).radiusAtSkill0, base.radiusAtSkill0, 0.0F, "scaling can be turned off");

        lrb::Config zero = base;
        zero.difficultyStrength = 0.0F;
        test::Near(lrb::ApplyDifficulty(zero, 4).speedAtSkill0, base.speedAtSkill0, 1e-6F, "strength 0 is neutral");

        lrb::Config steep = base;
        steep.difficultyStrength = 2.0F;
        test::Expect(lrb::ApplyDifficulty(steep, 4).radiusAtSkill0 < lrb::ApplyDifficulty(base, 4).radiusAtSkill0, "strength 2 is steeper");
        test::Expect(lrb::ApplyDifficulty(steep, 4).radiusAtSkill0 > 0.0F, "steep scaling never reaches zero");
    }
    {
        test::Near(lrb::ApplyDifficulty(base, -1).radiusAtSkill0, base.radiusAtSkill0, 0.0F, "unknown level is neutral");
        test::Near(lrb::ApplyDifficulty(base, 7).radiusAtSkill0, base.radiusAtSkill0, 0.0F, "out of range level is neutral");
        test::Expect(std::string(lrb::LockLevelName(4)) == "master", "level names");
    }
    return test::Finish("DifficultyTests");
}
