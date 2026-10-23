#pragma once

#include "buffer/RingBuffer.h"
#include "buffer/VisualizerSPSCQueue.h"
#include "dsp/DSPChain.h"
#include <portaudio.h>
#include <vector>
#include <utility>
#include <atomic>
#include <cstddef>
#include <string>

namespace AudioEngine
{
    class Gain;
    class Delay;
    class Equalizer;
    class LowPassFilter;
    class HighPassFilter;

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

        float GetGain() const
        {
            return m_gainValue.load(std::memory_order_relaxed);
        }

        float GetLowPassAlpha() const
        {
            return m_lowPassAlphaValue.load(std::memory_order_relaxed);
        }

        float GetHighPassAlpha() const
        {
            return m_highPassAlphaValue.load(std::memory_order_relaxed);
        }

        float GetDelayFeedback() const
        {
            return m_delayFeedbackValue.load(std::memory_order_relaxed);
        }

        float GetEqLowGain() const
        {
            return m_eqLowGain.load(std::memory_order_relaxed);
        }

        float GetEqMidGain() const
        {
            return m_eqMidGain.load(std::memory_order_relaxed);
        }

        float GetEqHighGain() const
        {
            return m_eqHighGain.load(std::memory_order_relaxed);
        }

        const std::vector<float>& GetVisualizerData();

        // Pass dynamic wiring down to the graph
        void SetGraphLinks(const std::vector<std::pair<int, int>>& links);
        bool AddDSPModule(int nodeId, const std::string& moduleName);

        int ProcessAudio(const void* input, void* output, unsigned long frameCount);

    private:
        RingBuffer<float> m_ringBuffer;
        DSPChain m_dspChain;
        PaStream* m_stream;

        Gain* m_activeGain;
        LowPassFilter* m_activeLowPass;
        HighPassFilter* m_activeHighPass;
        Delay* m_activeDelay;
        Equalizer* m_activeEqualizer;

        bool m_gainEnabled;
        bool m_lowPassEnabled;
        bool m_highPassEnabled;
        bool m_delayEnabled;
        bool m_equalizerEnabled;

        void ApplyPendingParameters();

        // These cached values are accessed by the audio callback only.
        float m_lastAppliedGain;
        float m_lastAppliedLowPassAlpha;
        float m_lastAppliedHighPassAlpha;
        float m_lastAppliedDelayFeedback;
        float m_lastAppliedEqLowGain;
        float m_lastAppliedEqMidGain;
        float m_lastAppliedEqHighGain;

        std::atomic<float> m_gainValue;
        std::atomic<float> m_lowPassAlphaValue;
        std::atomic<float> m_highPassAlphaValue;
        std::atomic<int> m_delaySamplesValue;
        std::atomic<float> m_delayFeedbackValue;

        std::atomic<float> m_eqLowGain;
        std::atomic<float> m_eqMidGain;
        std::atomic<float> m_eqHighGain;

        VisualizerSPSCQueue m_visualizerQueue;

        // These buffers are accessed only by the visualizer consumer.
        std::vector<float> m_visualizerHistory;
        std::vector<float> m_visualizerData;
        std::size_t m_visualizerWriteIndex;
    };
}