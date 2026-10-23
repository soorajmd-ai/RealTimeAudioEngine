
#include "io/AudioStream.h"
#include "Logger.h"
#include "dsp/Gain.h"
#include "dsp/LowPassFilter.h"
#include "dsp/HighPassFilter.h"
#include "dsp/Delay.h"
#include "dsp/Equalizer.h"
#include "dsp/DSPFactory.h"
#include "dsp/DSPModule.h"
#include <string>
#include <utility>

#include <memory>
#include <algorithm>

namespace AudioEngine
{
    static int AudioCallback(
        const void* input,
        void* output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData)
    {
        (void)timeInfo;
        (void)statusFlags;

        AudioStream* audioStream =
            static_cast<AudioStream*>(userData);

        return audioStream->ProcessAudio(input, output, frameCount);
    }

    AudioStream::AudioStream()
        : m_stream(nullptr),
        m_ringBuffer(8192),
        m_activeGain(nullptr),
        m_activeLowPass(nullptr),
        m_activeHighPass(nullptr),
        m_activeDelay(nullptr),
        m_activeEqualizer(nullptr),
        m_gainEnabled(true),
        m_lowPassEnabled(true),
        m_highPassEnabled(true),
        m_delayEnabled(true),
        m_equalizerEnabled(true),
        m_gainValue(1.0f),
        m_lowPassAlphaValue(0.2f),
        m_highPassAlphaValue(0.95f),
        m_delaySamplesValue(12000),
        m_delayFeedbackValue(0.5f),
        m_eqLowGain(0.0f),
        m_eqMidGain(0.0f),
        m_eqHighGain(0.0f),
        m_lastAppliedGain(1.0f),
        m_lastAppliedLowPassAlpha(0.2f),
        m_lastAppliedHighPassAlpha(0.95f),
        m_lastAppliedDelayFeedback(0.5f),
        m_lastAppliedEqLowGain(0.0f),
        m_lastAppliedEqMidGain(0.0f),
        m_lastAppliedEqHighGain(0.0f),
        m_visualizerHistory(512, 0.0f),
        m_visualizerData(512, 0.0f),
        m_visualizerWriteIndex(0)
    {}

    AudioStream::~AudioStream()
    {
        Close();
    }

    // ---------------------------------------------------------
    // Thread-safe parameter setters
    //
    // These functions only publish new values. They do not
    // directly modify DSP modules from the calling thread.
    // ---------------------------------------------------------

    void AudioStream::SetGain(float gain)
    {
        m_gainValue.store(gain, std::memory_order_relaxed);
    }

    void AudioStream::SetLowPassAlpha(float alpha)
    {
        m_lowPassAlphaValue.store(alpha, std::memory_order_relaxed);
    }

    void AudioStream::SetHighPassAlpha(float alpha)
    {
        m_highPassAlphaValue.store(alpha, std::memory_order_relaxed);
    }

    void AudioStream::SetDelaySamples(int samples)
    {
        m_delaySamplesValue.store(samples, std::memory_order_relaxed);
    }

    void AudioStream::SetDelayFeedback(float feedback)
    {
        m_delayFeedbackValue.store(feedback, std::memory_order_relaxed);
    }

    void AudioStream::SetEqLowGain(float dbGain)
    {
        m_eqLowGain.store(dbGain, std::memory_order_relaxed);
    }

    void AudioStream::SetEqMidGain(float dbGain)
    {
        m_eqMidGain.store(dbGain, std::memory_order_relaxed);
    }

    void AudioStream::SetEqHighGain(float dbGain)
    {
        m_eqHighGain.store(dbGain, std::memory_order_relaxed);
    }

    void AudioStream::EnableGain(bool enable)
    {
        m_gainEnabled = enable;
    }

    void AudioStream::EnableLowPass(bool enable)
    {
        m_lowPassEnabled = enable;
    }

    void AudioStream::EnableHighPass(bool enable)
    {
        m_highPassEnabled = enable;
    }

    void AudioStream::EnableDelay(bool enable)
    {
        m_delayEnabled = enable;
    }

    void AudioStream::EnableEqualizer(bool enable)
    {
        m_equalizerEnabled = enable;
    }

    void AudioStream::SetGraphLinks(
        const std::vector<std::pair<int, int>>& links)
    {
        m_dspChain.SetLinks(links);
    }

    bool AudioStream::AddDSPModule(
        int nodeId,
        const std::string& moduleName)
    {
        if (nodeId <= 1 || nodeId == 6)
        {
            return false;
        }

        std::unique_ptr<DSPModule> module =
            DSPFactory::Create(moduleName);

        if (!module)
        {
            return false;
        }

        m_dspChain.AddModule(nodeId, std::move(module));
        return true;
    }
    // ---------------------------------------------------------
    // Create and configure the DSP modules
    // ---------------------------------------------------------

    bool AudioStream::OpenDefaultStream()
    {
        Logger::Info("Opening default audio stream...");

        // The graph determines which modules are processed.
        // All available modules are therefore registered here.

        auto gain = std::make_unique<Gain>();
        m_activeGain = gain.get();

        const float initialGain =
            m_gainValue.load(std::memory_order_relaxed);

        m_activeGain->SetGain(initialGain);
        m_lastAppliedGain = initialGain;

        m_dspChain.AddModule(2, std::move(gain));

        auto lowPass = std::make_unique<LowPassFilter>();
        m_activeLowPass = lowPass.get();

        const float initialLowPassAlpha =
            m_lowPassAlphaValue.load(std::memory_order_relaxed);

        m_activeLowPass->SetAlpha(initialLowPassAlpha);
        m_lastAppliedLowPassAlpha = initialLowPassAlpha;

        m_dspChain.AddModule(3, std::move(lowPass));

        auto highPass = std::make_unique<HighPassFilter>();
        m_activeHighPass = highPass.get();

        const float initialHighPassAlpha =
            m_highPassAlphaValue.load(std::memory_order_relaxed);

        m_activeHighPass->SetAlpha(initialHighPassAlpha);
        m_lastAppliedHighPassAlpha = initialHighPassAlpha;

        m_dspChain.AddModule(4, std::move(highPass));

        auto eq = std::make_unique<Equalizer>();
        m_activeEqualizer = eq.get();

        const float initialEqLowGain =
            m_eqLowGain.load(std::memory_order_relaxed);

        const float initialEqMidGain =
            m_eqMidGain.load(std::memory_order_relaxed);

        const float initialEqHighGain =
            m_eqHighGain.load(std::memory_order_relaxed);

        m_activeEqualizer->SetLowGain(initialEqLowGain);
        m_activeEqualizer->SetMidGain(initialEqMidGain);
        m_activeEqualizer->SetHighGain(initialEqHighGain);

        m_lastAppliedEqLowGain = initialEqLowGain;
        m_lastAppliedEqMidGain = initialEqMidGain;
        m_lastAppliedEqHighGain = initialEqHighGain;

        m_dspChain.AddModule(7, std::move(eq));

        auto delay = std::make_unique<Delay>();
        m_activeDelay = delay.get();

        const int initialDelaySamples =
            m_delaySamplesValue.load(std::memory_order_relaxed);

        const float initialDelayFeedback =
            m_delayFeedbackValue.load(std::memory_order_relaxed);

        m_activeDelay->SetDelaySamples(initialDelaySamples);
        m_activeDelay->SetFeedback(initialDelayFeedback);

        m_lastAppliedDelayFeedback = initialDelayFeedback;

        m_dspChain.AddModule(5, std::move(delay));

        const PaError error = Pa_OpenDefaultStream(
            &m_stream,
            1,
            1,
            paFloat32,
            48000,
            512,
            AudioCallback,
            this);

        if (error != paNoError)
        {
            Logger::Error("Failed to open default audio stream.");
            m_stream = nullptr;
            return false;
        }

        Logger::Info("Audio stream opened successfully.");
        return true;
    }

    bool AudioStream::Start()
    {
        if (m_stream == nullptr)
        {
            Logger::Error("Cannot start audio stream: stream is null.");
            return false;
        }

        return Pa_StartStream(m_stream) == paNoError;
    }

    void AudioStream::Stop()
    {
        if (m_stream != nullptr && Pa_IsStreamActive(m_stream) == 1)
        {
            Pa_StopStream(m_stream);
        }
    }

    void AudioStream::Close()
    {
        if (m_stream != nullptr)
        {
            Stop();
            Pa_CloseStream(m_stream);
            m_stream = nullptr;
        }
    }

    // ---------------------------------------------------------
    // Apply published parameter changes on the audio thread
    // ---------------------------------------------------------

    void AudioStream::ApplyPendingParameters()
    {
        const float gain =
            m_gainValue.load(std::memory_order_relaxed);

        if (gain != m_lastAppliedGain)
        {
            if (m_activeGain != nullptr)
            {
                m_activeGain->SetGain(gain);
            }

            m_lastAppliedGain = gain;
        }

        const float lowPassAlpha =
            m_lowPassAlphaValue.load(std::memory_order_relaxed);

        if (lowPassAlpha != m_lastAppliedLowPassAlpha)
        {
            if (m_activeLowPass != nullptr)
            {
                m_activeLowPass->SetAlpha(lowPassAlpha);
            }

            m_lastAppliedLowPassAlpha = lowPassAlpha;
        }

        const float highPassAlpha =
            m_highPassAlphaValue.load(std::memory_order_relaxed);

        if (highPassAlpha != m_lastAppliedHighPassAlpha)
        {
            if (m_activeHighPass != nullptr)
            {
                m_activeHighPass->SetAlpha(highPassAlpha);
            }

            m_lastAppliedHighPassAlpha = highPassAlpha;
        }

        const float delayFeedback =
            m_delayFeedbackValue.load(std::memory_order_relaxed);

        if (delayFeedback != m_lastAppliedDelayFeedback)
        {
            if (m_activeDelay != nullptr)
            {
                m_activeDelay->SetFeedback(delayFeedback);
            }

            m_lastAppliedDelayFeedback = delayFeedback;
        }

        const float eqLowGain =
            m_eqLowGain.load(std::memory_order_relaxed);

        if (eqLowGain != m_lastAppliedEqLowGain)
        {
            if (m_activeEqualizer != nullptr)
            {
                m_activeEqualizer->SetLowGain(eqLowGain);
            }

            m_lastAppliedEqLowGain = eqLowGain;
        }

        const float eqMidGain =
            m_eqMidGain.load(std::memory_order_relaxed);

        if (eqMidGain != m_lastAppliedEqMidGain)
        {
            if (m_activeEqualizer != nullptr)
            {
                m_activeEqualizer->SetMidGain(eqMidGain);
            }

            m_lastAppliedEqMidGain = eqMidGain;
        }

        const float eqHighGain =
            m_eqHighGain.load(std::memory_order_relaxed);

        if (eqHighGain != m_lastAppliedEqHighGain)
        {
            if (m_activeEqualizer != nullptr)
            {
                m_activeEqualizer->SetHighGain(eqHighGain);
            }

            m_lastAppliedEqHighGain = eqHighGain;
        }
    }

    // ---------------------------------------------------------
    // Consumer-side visualizer snapshot
    // ---------------------------------------------------------

    const std::vector<float>& AudioStream::GetVisualizerData()
    {
        float sample = 0.0f;

        // Only the UI/consumer thread drains this queue and updates
        // the rolling history.
        while (m_visualizerQueue.Pop(sample))
        {
            m_visualizerHistory[m_visualizerWriteIndex] = sample;

            m_visualizerWriteIndex =
                (m_visualizerWriteIndex + 1) %
                m_visualizerHistory.size();
        }

        // Present the history in chronological order.
        // m_visualizerWriteIndex points to the oldest sample.
        for (std::size_t i = 0; i < m_visualizerData.size(); ++i)
        {
            m_visualizerData[i] =
                m_visualizerHistory[
                    (m_visualizerWriteIndex + i) %
                    m_visualizerHistory.size()];
        }

        return m_visualizerData;
    }

    // ---------------------------------------------------------
    // Real-time audio callback processing
    // ---------------------------------------------------------

    int AudioStream::ProcessAudio(
        const void* input,
        void* output,
        unsigned long frameCount)
    {
        // Apply graph changes once per block without waiting for the UI.
        m_dspChain.BeginAudioBlock();

        // Only the audio callback applies new DSP parameters.
        ApplyPendingParameters();

        const float* inputBuffer =
            static_cast<const float*>(input);

        float* outputBuffer =
            static_cast<float*>(output);

        if (outputBuffer == nullptr)
        {
            return paAbort;
        }

        if (inputBuffer == nullptr)
        {
            for (unsigned long i = 0; i < frameCount; ++i)
            {
                outputBuffer[i] = 0.0f;
                (void)m_visualizerQueue.Push(0.0f);
            }

            return paContinue;
        }

        for (unsigned long i = 0; i < frameCount; ++i)
        {
            m_ringBuffer.push(inputBuffer[i]);
        }

        for (unsigned long i = 0; i < frameCount; ++i)
        {
            float sample = 0.0f;

            if (m_ringBuffer.pop(sample))
            {
                outputBuffer[i] = m_dspChain.Process(sample);
            }
            else
            {
                outputBuffer[i] = 0.0f;
            }
        }

        // Publish samples without touching the UI-owned vectors.
        // If the queue is full, drop samples rather than blocking audio.
        for (unsigned long i = 0; i < frameCount; ++i)
        {
            (void)m_visualizerQueue.Push(outputBuffer[i]);
        }

        return paContinue;
    }

} // namespace AudioEngine
