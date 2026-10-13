#include "dsp/HighPassFilter.h"

namespace AudioEngine
{

    HighPassFilter::HighPassFilter()
        :
        m_alpha(0.95f),
        m_previousInput(0.0f),
        m_previousOutput(0.0f)
    {}

    void HighPassFilter::SetAlpha(float alpha)
    {
        m_alpha = alpha;
    }

    float HighPassFilter::Process(float sample)
    {
        float output =
            m_alpha *
            (m_previousOutput +
                sample -
                m_previousInput);

        m_previousInput = sample;

        m_previousOutput = output;

        return output;
    }

}