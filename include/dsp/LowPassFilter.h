#pragma once

#include "dsp/DSPModule.h"

namespace AudioEngine
{
    class LowPassFilter : public DSPModule
    {
    public:

        LowPassFilter();

        void SetAlpha(float alpha);

        float Process(float sample) override;

    private:

        float m_alpha;
        float m_previousOutput;
    };
}
