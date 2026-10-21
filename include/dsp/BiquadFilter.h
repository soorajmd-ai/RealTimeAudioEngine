#pragma once
#include "dsp/DSPModule.h"

namespace AudioEngine
{
    class BiquadFilter : public DSPModule
    {
    public:
        BiquadFilter();

        // Audio EQ Cookbook standard shapes
        void SetLowShelf(float sampleRate, float cutoffFreq, float Q, float dbGain);
        void SetHighShelf(float sampleRate, float cutoffFreq, float Q, float dbGain);
        void SetPeaking(float sampleRate, float centerFreq, float Q, float dbGain);

        float Process(float sample) override;

    private:
        // Feedforward and Feedback coefficients
        float m_b0, m_b1, m_b2;
        float m_a1, m_a2;

        // Sample history (state variables)
        float m_x1, m_x2;
        float m_y1, m_y2;
    };
}