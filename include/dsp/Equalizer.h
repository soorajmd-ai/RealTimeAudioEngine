#pragma once

#include "dsp/DSPModule.h"
#include "dsp/LowPassFilter.h"
#include "dsp/HighPassFilter.h"
#include "dsp/Gain.h"

namespace AudioEngine
{
    class Equalizer : public DSPModule
    {
    public:

        Equalizer();

        void SetBassGain(float gain);

        void SetMidGain(float gain);

        void SetTrebleGain(float gain);

        float Process(float sample) override;

    private:

        LowPassFilter m_lowPass;

        HighPassFilter m_highPass;

        Gain m_bassGain;

        Gain m_midGain;

        Gain m_trebleGain;
    };
}
