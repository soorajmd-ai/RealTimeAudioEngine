#include "dsp/Equalizer.h"

namespace AudioEngine
{

    Equalizer::Equalizer()
    {
        // Initialize bands with a flat 0dB response at 48kHz
        m_lowBand.SetLowShelf(48000.0f, 250.0f, 0.707f, 0.0f);
        m_midBand.SetPeaking(48000.0f, 1000.0f, 0.707f, 0.0f);
        m_highBand.SetHighShelf(48000.0f, 4000.0f, 0.707f, 0.0f);
    }

    void Equalizer::SetLowGain(float dbGain)
    {
        m_lowBand.SetLowShelf(48000.0f, 250.0f, 0.707f, dbGain);
    }

    void Equalizer::SetMidGain(float dbGain)
    {
        m_midBand.SetPeaking(48000.0f, 1000.0f, 0.707f, dbGain);
    }

    void Equalizer::SetHighGain(float dbGain)
    {
        m_highBand.SetHighShelf(48000.0f, 4000.0f, 0.707f, dbGain);
    }

    float Equalizer::Process(float sample)
    {
        // Pass the sample sequentially through all three bands
        sample = m_lowBand.Process(sample);
        sample = m_midBand.Process(sample);
        sample = m_highBand.Process(sample);

        return sample;
    }

}