#include "dsp/BiquadFilterTest.h"
#include "dsp/BiquadFilter.h"

#include <cmath>
#include <iostream>

namespace AudioEngine
{
    bool RunBiquadFilterTests()
    {
        std::cout << "\nRunning BiquadFilter tests...\n";

        BiquadFilter filter(48000.0f, 1000.0f);

        // Test 1: Invalid cutoff frequency
        if (filter.SetLowPass(48000.0f, 24000.0f))
        {
            std::cout << "FAIL: Invalid cutoff was accepted.\n";
            return false;
        }

        std::cout << "PASS: Invalid cutoff was rejected.\n";

        // Test 2: Impulse response
        filter.Reset();

        float firstOutput = 0.0f;
        float lateOutput = 0.0f;

        for (int i = 0; i < 500; ++i)
        {
            const float input = (i == 0) ? 1.0f : 0.0f;
            const float output = filter.Process(input);

            if (!std::isfinite(output))
            {
                std::cout << "FAIL: Non-finite output detected.\n";
                return false;
            }

            if (i == 0)
            {
                firstOutput = output;
            }

            if (i == 499)
            {
                lateOutput = output;
            }
        }

        if (firstOutput <= 0.0f ||
            std::fabs(lateOutput) >= 0.0001f)
        {
            std::cout << "FAIL: Unexpected impulse response.\n";
            return false;
        }

        std::cout << "PASS: Impulse response is finite and decays.\n";
        std::cout << "First output: " << firstOutput << '\n';
        std::cout << "Output after 500 samples: " << lateOutput << '\n';

        std::cout << "All BiquadFilter tests passed.\n";
        return true;
    }
}