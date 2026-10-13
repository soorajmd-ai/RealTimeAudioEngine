#include "dsp/Equalizer.h"

namespace AudioEngine
{

    Equalizer::Equalizer()
    {
        m_lowPass.SetAlpha(0.15f);

        m_highPass.SetAlpha(0.15f);

        m_bassGain.SetGain(1.0f);

        m_midGain.SetGain(1.0f);

        m_trebleGain.SetGain(1.0f);
    }

    void Equalizer::SetBassGain(float gain)
    {
        m_bassGain.SetGain(gain);
    }

    void Equalizer::SetMidGain(float gain)
    {
        m_midGain.SetGain(gain);
    }

    void Equalizer::SetTrebleGain(float gain)
    {
        m_trebleGain.SetGain(gain);
    }

    float Equalizer::Process(float sample)
    {
        float bass =
            m_lowPass.Process(sample);

        float treble =
            m_highPass.Process(sample);

        float mid =
            sample - bass - treble;

        bass =
            m_bassGain.Process(bass);

        mid =
            m_midGain.Process(mid);

        treble =
            m_trebleGain.Process(treble);

        return
            bass +
            mid +
            treble;
    }

}