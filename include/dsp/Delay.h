#pragma once

#include "dsp/DSPModule.h"

#include <vector>

namespace AudioEngine
{
    class Delay : public DSPModule
    {
    public:

        Delay();

        void SetDelaySamples(int delaySamples);

        void SetFeedback(float feedback);

        float Process(float sample) override;

    private:

        std::vector<float> m_buffer; // stores old audio samples.

        int m_writeIndex; // tells us where to write the next sample.

        int m_delaySamples; // how far back we read to create the delay.

        float m_feedback; // controls the amount of delayed signal fed back into the delay line.
    };
}
