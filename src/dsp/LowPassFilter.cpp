#include "dsp/LowPassFilter.h"

namespace AudioEngine
{

    LowPassFilter::LowPassFilter()
        :
        m_alpha(0.2f),
        m_previousOutput(0.0f)
    {}

    void LowPassFilter::SetAlpha(float alpha)
    {
        m_alpha = alpha;
    }

    float LowPassFilter::Process(float sample)
    {
        m_previousOutput =
            m_alpha * sample +
            (1.0f - m_alpha) * m_previousOutput;

        return m_previousOutput;
    }

}