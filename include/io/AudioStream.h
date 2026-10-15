#pragma once

#include "buffer/RingBuffer.h"
#include "dsp/DSPChain.h"
#include <portaudio.h>

namespace AudioEngine
{
    class Gain;
    class Delay;
    class Equalizer;

    class AudioStream
    {
    public:
        AudioStream();
        ~AudioStream();

        bool OpenDefaultStream();
        bool Start();
        void Stop();
        void Close();

        void SetGain(float gain);
        void SetLowPassAlpha(float alpha);
        void SetHighPassAlpha(float alpha);
        void SetDelaySamples(int samples);
        void SetDelayFeedback(float feedback);

        // Equalizer Setters
        void SetEqLowGain(float dbGain);
        void SetEqMidGain(float dbGain);
        void SetEqHighGain(float dbGain);

        void EnableGain(bool enable);
        void EnableLowPass(bool enable);
        void EnableHighPass(bool enable);
        void EnableDelay(bool enable);
        void EnableEqualizer(bool enable);

        // Getters for the GUI
        float GetGain() const { return m_gainValue; }
        float GetDelayFeedback() const { return m_delayFeedbackValue; }
        float GetEqLowGain() const { return m_eqLowGain; }
        float GetEqMidGain() const { return m_eqMidGain; }
        float GetEqHighGain() const { return m_eqHighGain; }

        int ProcessAudio(const void* input, void* output, unsigned long frameCount);

    private:
        RingBuffer<float> m_ringBuffer;
        DSPChain m_dspChain;
        PaStream* m_stream;

        // Observer pointers for real-time GUI control
        Gain* m_activeGain;
        Delay* m_activeDelay;
        Equalizer* m_activeEqualizer;

        // Enable flags
        bool m_gainEnabled;
        bool m_lowPassEnabled;
        bool m_highPassEnabled;
        bool m_delayEnabled;
        bool m_equalizerEnabled;

        // Parameter values
        float m_gainValue;
        float m_lowPassAlphaValue;
        float m_highPassAlphaValue;
        int m_delaySamplesValue;
        float m_delayFeedbackValue;

        // Equalizer values (in Decibels)
        float m_eqLowGain;
        float m_eqMidGain;
        float m_eqHighGain;
    };
}