#include "Core/RevealField.h"
#include "TestUtil.h"

int main()
{
    lrb::Config config;

    {
        const auto low = lrb::ComputeRevealParams(config, 0.0F, false);
        const auto high = lrb::ComputeRevealParams(config, 100.0F, false);
        const auto over = lrb::ComputeRevealParams(config, 250.0F, false);
        test::Near(low.radius, config.radiusAtSkill0, 1e-5F, "radius at skill 0");
        test::Near(high.radius, config.radiusAtSkill100, 1e-5F, "radius at skill 100");
        test::Near(over.rate, high.rate, 1e-5F, "skill above cap is clamped");
        test::Expect(high.rate > low.rate && high.radius > low.radius, "higher skill reveals wider and faster");

        const auto turning = lrb::ComputeRevealParams(config, 50.0F, true);
        const auto still = lrb::ComputeRevealParams(config, 50.0F, false);
        test::Near(turning.rate, still.rate * config.turningSpeedMult, 1e-5F, "turning multiplier");

        lrb::Config onlyTurning = config;
        onlyTurning.revealOnlyWhileTurning = true;
        test::Near(lrb::ComputeRevealParams(onlyTurning, 50.0F, false).rate, 0.0F, 0.0F, "no reveal without tension");
    }

    {
        lrb::RevealField field;
        field.Reset(180);
        test::Near(field.CellMin(0), -90.0F, 1e-5F, "first cell starts at -90");
        test::Near(field.CellMax(179), 90.0F, 1e-4F, "last cell ends at +90");

        const lrb::RevealParams params{ 5.0F, 1.0F };
        test::Expect(field.Step(0.5F, params, 0.5F, 0.0F), "step reports change");
        test::Near(field.Get(90), 0.5F, 1e-5F, "cell under the pick gains rate*dt");
        test::Expect(field.Get(92) > 0.0F && field.Get(92) < field.Get(90), "neighbour gains less");
        test::Near(field.Get(100), 0.0F, 0.0F, "cell outside radius untouched");
        test::Near(field.Get(0), 0.0F, 0.0F, "far cell stays black");

        for (int i = 0; i < 10; ++i) {
            field.Step(0.5F, params, 0.5F, 0.0F);
        }
        test::Near(field.Get(90), 1.0F, 0.0F, "reveal saturates at 1");

        field.Step(-60.0F, lrb::RevealParams{ 1.0F, 0.0F }, 0.25F, 1.0F);
        test::Near(field.Get(90), 0.75F, 1e-5F, "fade dims cells away from the pick");

        test::Expect(!field.Step(-60.0F, lrb::RevealParams{ 1.0F, 0.0F }, 0.25F, 0.0F), "no change without rate or fade");
    }
    return test::Finish("RevealFieldTests");
}
