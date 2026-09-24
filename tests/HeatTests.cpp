#include "Core/Heat.h"
#include "TestUtil.h"

int main()
{
    const lrb::LockGeometry lock{ 10.0F, 4.0F, 12.0F };  // sweet [8,12], partial out to 24 / -4

    test::Near(lrb::Heat(lock, 10.0F, 1.6F), 1.0F, 1e-6F, "center is sweet");
    test::Near(lrb::Heat(lock, 12.0F, 1.6F), 1.0F, 1e-6F, "sweet edge inclusive");
    test::Near(lrb::Heat(lock, 12.001F, 1.6F), lrb::kPartialHigh, 1e-3F, "partial starts at its high end");
    test::Near(lrb::Heat(lock, 24.0F, 1.6F), lrb::kPartialLow, 1e-4F, "partial ends at its low end");
    test::Expect(lrb::Heat(lock, 24.01F, 1.6F) <= lrb::kFarHigh + 1e-6F, "far zone below partial");
    test::Expect(lrb::Heat(lock, 30.0F, 1.6F) > lrb::Heat(lock, 60.0F, 1.6F), "far zone decreases with distance");
    test::Near(lrb::Heat(lock, -4.0F, 1.6F), lrb::Heat(lock, 24.0F, 1.6F), 1e-4F, "symmetric around center");

    // A 1-degree cell that only partly overlaps a narrow sweet spot must still read as sweet.
    const lrb::LockGeometry master{ 0.35F, 0.5F, 6.0F };
    test::Near(lrb::CellHeat(master, 0.0F, 1.0F, 1.6F), 1.0F, 1e-6F, "narrow sweet spot inside cell");
    test::Expect(lrb::CellContainsSweetSpot(master, 0.0F, 1.0F), "cell contains sweet spot");
    test::Expect(!lrb::CellContainsSweetSpot(master, 1.0F, 2.0F), "next cell does not");

    const lrb::LockGeometry noPartial{ 0.0F, 2.0F, 0.0F };
    test::Expect(lrb::Heat(noPartial, 1.5F, 1.6F) <= lrb::kFarHigh, "zero partial falls straight to far zone");
    return test::Finish("HeatTests");
}
