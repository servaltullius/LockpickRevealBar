#include "Core/Effects.h"
#include "TestUtil.h"

int main()
{
    lrb::BarEffects fx;
    test::Expect(!fx.Active(), "idle at start");
    test::Near(fx.SweetFlashAlpha(), 0.0F, 0.0F, "no flash at start");
    test::Near(fx.ShakeOffset(), 0.0F, 0.0F, "no shake at start");

    fx.TriggerSweetFlash();
    test::Expect(fx.Active(), "active after trigger");
    const float start = fx.SweetFlashAlpha();
    fx.Advance(0.3F);
    test::Expect(fx.SweetFlashAlpha() < start && fx.SweetFlashAlpha() > 0.0F, "flash fades");
    fx.Advance(1.0F);
    test::Near(fx.SweetFlashAlpha(), 0.0F, 0.0F, "flash ends");
    test::Expect(!fx.Active(), "idle after the flash");

    fx.TriggerBreak(false);
    test::Expect(fx.BreakFlashAlpha() > 0.0F, "break flash");
    test::Near(fx.ShakeOffset(), 0.0F, 0.0F, "no shake when disabled");

    fx.TriggerBreak(true);
    fx.Advance(0.02F);
    test::Expect(fx.ShakeOffset() != 0.0F, "shake moves the bar");
    test::Expect(fx.ShakeOffset() >= -1.0F && fx.ShakeOffset() <= 1.0F, "shake stays within the amplitude");
    fx.Advance(1.0F);
    test::Near(fx.ShakeOffset(), 0.0F, 0.0F, "shake settles");
    test::Near(fx.BreakFlashAlpha(), 0.0F, 0.0F, "break flash ends");
    return test::Finish("EffectsTests");
}
