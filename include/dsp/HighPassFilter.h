#pragma once

#include "dsp/DSPModule.h"

namespace AudioEngine
{
    class HighPassFilter : public DSPModule
    {
    public:

        HighPassFilter();

        void SetAlpha(float alpha);

        float Process(float sample) override;

    private:

        float m_alpha;

        float m_previousInput;

        float m_previousOutput;
    };
}
