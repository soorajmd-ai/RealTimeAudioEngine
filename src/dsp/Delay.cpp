#include "dsp/Delay.h"

namespace AudioEngine
{

    Delay::Delay()
        :
        m_writeIndex(0),
        m_delaySamples(24000),
        m_feedback(0.5f)
    {
        m_buffer.resize(48000, 0.0f);
    }

    void Delay::SetDelaySamples(int delaySamples)
    {
        m_delaySamples = delaySamples;
    }

    void Delay::SetFeedback(float feedback)
    {
        m_feedback = feedback;
    }

    float Delay::Process(float sample)
    {
        int readIndex = m_writeIndex - m_delaySamples;

        if (readIndex < 0)
        {
            readIndex += static_cast<int>(m_buffer.size());
        }

        float delayedSample = m_buffer[readIndex];

        m_buffer[m_writeIndex] = sample + delayedSample * m_feedback;

        m_writeIndex++;

        if (m_writeIndex >= static_cast<int>(m_buffer.size()))
        {
            m_writeIndex = 0;
        }

        return sample + delayedSample * 0.5f;
    }

}