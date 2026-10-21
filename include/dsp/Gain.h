#pragma once
#include "dsp/DSPModule.h"

namespace AudioEngine
{
    class Gain : public DSPModule
    {
    public:

        Gain();

        void SetGain(float gain);

        float Process(float sample) override;

    private:

        float m_gain;
    };
}
