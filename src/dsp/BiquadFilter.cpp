#include "dsp/BiquadFilter.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace AudioEngine
{

    BiquadFilter::BiquadFilter()
        : m_b0(1.0f), m_b1(0.0f), m_b2(0.0f),
        m_a1(0.0f), m_a2(0.0f),
        m_x1(0.0f), m_x2(0.0f),
        m_y1(0.0f), m_y2(0.0f)
    {}

    void BiquadFilter::SetLowShelf(float sampleRate, float cutoffFreq, float Q, float dbGain)
    {
        float A = std::pow(10.0f, dbGain / 40.0f);
        float w0 = 2.0f * static_cast<float>(M_PI) * cutoffFreq / sampleRate;
        float alpha = std::sin(w0) / (2.0f * Q);
        float cosw0 = std::cos(w0);

        float b0 = A * ((A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha);
        float b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosw0);
        float b2 = A * ((A + 1.0f) - (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha);
        float a0 = (A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha;
        float a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cosw0);
        float a2 = (A + 1.0f) + (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha;

        m_b0 = b0 / a0;
        m_b1 = b1 / a0;
        m_b2 = b2 / a0;
        m_a1 = a1 / a0;
        m_a2 = a2 / a0;
    }

    void BiquadFilter::SetHighShelf(float sampleRate, float cutoffFreq, float Q, float dbGain)
    {
        float A = std::pow(10.0f, dbGain / 40.0f);
        float w0 = 2.0f * static_cast<float>(M_PI) * cutoffFreq / sampleRate;
        float alpha = std::sin(w0) / (2.0f * Q);
        float cosw0 = std::cos(w0);

        float b0 = A * ((A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha);
        float b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosw0);
        float b2 = A * ((A + 1.0f) + (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha);
        float a0 = (A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha;
        float a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosw0);
        float a2 = (A + 1.0f) - (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha;

        m_b0 = b0 / a0;
        m_b1 = b1 / a0;
        m_b2 = b2 / a0;
        m_a1 = a1 / a0;
        m_a2 = a2 / a0;
    }

    void BiquadFilter::SetPeaking(float sampleRate, float centerFreq, float Q, float dbGain)
    {
        float A = std::pow(10.0f, dbGain / 40.0f);
        float w0 = 2.0f * static_cast<float>(M_PI) * centerFreq / sampleRate;
        float alpha = std::sin(w0) / (2.0f * Q);
        float cosw0 = std::cos(w0);

        float b0 = 1.0f + alpha * A;
        float b1 = -2.0f * cosw0;
        float b2 = 1.0f - alpha * A;
        float a0 = 1.0f + alpha / A;
        float a1 = -2.0f * cosw0;
        float a2 = 1.0f - alpha / A;

        m_b0 = b0 / a0;
        m_b1 = b1 / a0;
        m_b2 = b2 / a0;
        m_a1 = a1 / a0;
        m_a2 = a2 / a0;
    }

    float BiquadFilter::Process(float sample)
    {
        // Biquad difference equation: y[n] = (b0*x[n] + b1*x[n-1] + b2*x[n-2]) - (a1*y[n-1] + a2*y[n-2])
        float output = (m_b0 * sample) + (m_b1 * m_x1) + (m_b2 * m_x2)
            - (m_a1 * m_y1) - (m_a2 * m_y2);

        // Shift state variables
        m_x2 = m_x1;
        m_x1 = sample;

        m_y2 = m_y1;
        m_y1 = output;

        return output;
    }

}