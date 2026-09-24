#include "Core/LockLayout.h"
#include "TestUtil.h"

#include <limits>

int main()
{
    using lrb::RawLockValues;
    test::Expect(lrb::IsPlausible(RawLockValues{ 12.0F, 7.5F, 18.0F }), "adept lock");
    test::Expect(lrb::IsPlausible(RawLockValues{ -88.0F, 3.0F, 10.0F }), "center near edge");
    test::Expect(!lrb::IsPlausible(RawLockValues{ 0.0F, 0.0F, 0.0F }), "unrolled lock");
    test::Expect(!lrb::IsPlausible(RawLockValues{ 89.9F, 3.0F, 10.0F }), "center outside the rolled range");
    test::Expect(!lrb::IsPlausible(RawLockValues{ 0.0F, 1e-40F, 10.0F }), "denormal width");
    test::Expect(!lrb::IsPlausible(RawLockValues{ 0.0F, 5.0F, 1e-40F }), "denormal partial");
    test::Expect(!lrb::IsPlausible(RawLockValues{ std::numeric_limits<float>::quiet_NaN(), 5.0F, 5.0F }), "NaN center");
    test::Expect(lrb::IsPlausible(RawLockValues{ 0.0F, 5.0F, 0.0F }), "zero partial allowed");

    using lrb::Layout;
    using lrb::LayoutOverride;
    test::Expect(lrb::ResolveLayout(LayoutOverride::kAuto, 1, 5) == Layout::kLegacy, "1.5.97 legacy");
    test::Expect(lrb::ResolveLayout(LayoutOverride::kAuto, 1, 6) == Layout::kLegacy, "1.6.1170 legacy");
    test::Expect(lrb::ResolveLayout(LayoutOverride::kAuto, 1, 7) == Layout::kShifted, "1.7.104 shifted");
    test::Expect(lrb::ResolveLayout(LayoutOverride::kLegacy, 1, 7) == Layout::kLegacy, "override legacy");
    test::Expect(lrb::ResolveLayout(LayoutOverride::kShifted, 1, 6) == Layout::kShifted, "override shifted");
    return test::Finish("LockLayoutTests");
}
