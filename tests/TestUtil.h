#pragma once

#include <cmath>
#include <iostream>

namespace test
{
    inline int failures = 0;

    inline void Expect(bool condition, const char* message)
    {
        if (!condition) {
            std::cerr << "FAIL: " << message << '\n';
            ++failures;
        }
    }

    inline void Near(float actual, float expected, float tolerance, const char* message)
    {
        if (std::fabs(actual - expected) > tolerance) {
            std::cerr << "FAIL: " << message << " (got " << actual << ", expected " << expected << ")\n";
            ++failures;
        }
    }

    inline int Finish(const char* suite)
    {
        if (failures == 0) {
            std::cout << suite << ": all passed\n";
            return 0;
        }
        std::cerr << suite << ": " << failures << " failure(s)\n";
        return 1;
    }
}
