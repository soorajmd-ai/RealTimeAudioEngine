#pragma once
#include "dsp/DSPModule.h"
#include "dsp/BiquadFilter.h"

namespace AudioEngine
{
    class Equalizer : public DSPModule
    {
    public:
        Equalizer();

        void SetLowGain(float dbGain);
        void SetMidGain(float dbGain);
        void SetHighGain(float dbGain);

        float Process(float sample) override;

    private:
        BiquadFilter m_lowBand;
        BiquadFilter m_midBand;
        BiquadFilter m_highBand;
    };
}