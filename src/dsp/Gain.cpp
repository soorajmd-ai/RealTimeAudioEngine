#include "dsp/Gain.h"

namespace AudioEngine
{

    Gain::Gain()
        :
        m_gain(1.0f)
    {}

    void Gain::SetGain(float gain)
    {
        m_gain = gain;
    }

    float Gain::Process(float sample)
    {
        return sample * m_gain;
    }

}