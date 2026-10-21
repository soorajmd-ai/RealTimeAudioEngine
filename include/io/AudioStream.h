#pragma once

#include "buffer/RingBuffer.h"
#include "dsp/DSPChain.h"
#include <portaudio.h>
#include <vector>

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

        void SetEqLowGain(float dbGain);
        void SetEqMidGain(float dbGain);
        void SetEqHighGain(float dbGain);

        void EnableGain(bool enable);
        void EnableLowPass(bool enable);
        void EnableHighPass(bool enable);
        void EnableDelay(bool enable);
        void EnableEqualizer(bool enable);

        float GetGain() const { return m_gainValue; }
        float GetDelayFeedback() const { return m_delayFeedbackValue; }
        float GetEqLowGain() const { return m_eqLowGain; }
        float GetEqMidGain() const { return m_eqMidGain; }
        float GetEqHighGain() const { return m_eqHighGain; }

        // Fetch the live waveform data
        const std::vector<float>& GetVisualizerData() const { return m_visualizerData; }

        int ProcessAudio(const void* input, void* output, unsigned long frameCount);

    private:
        RingBuffer<float> m_ringBuffer;
        DSPChain m_dspChain;
        PaStream* m_stream;

        Gain* m_activeGain;
        Delay* m_activeDelay;
        Equalizer* m_activeEqualizer;

        bool m_gainEnabled;
        bool m_lowPassEnabled;
        bool m_highPassEnabled;
        bool m_delayEnabled;
        bool m_equalizerEnabled;

        float m_gainValue;
        float m_lowPassAlphaValue;
        float m_highPassAlphaValue;
        int m_delaySamplesValue;
        float m_delayFeedbackValue;

        float m_eqLowGain;
        float m_eqMidGain;
        float m_eqHighGain;

        // Lock-free snapshot buffer for the UI Oscilloscope
        std::vector<float> m_visualizerData;
    };
}