#include "io/AudioStream.h"
#include "Logger.h"
#include "dsp/Gain.h"
#include "dsp/LowPassFilter.h"
#include "dsp/HighPassFilter.h"
#include "dsp/Delay.h"
#include "dsp/Equalizer.h"
#include <memory>

namespace AudioEngine
{

    static int AudioCallback(const void* input, void* output, unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
    {
        (void)timeInfo;
        (void)statusFlags;
        AudioStream* audioStream = static_cast<AudioStream*>(userData);
        return audioStream->ProcessAudio(input, output, frameCount);
    }

    AudioStream::AudioStream()
        : m_stream(nullptr),
        m_ringBuffer(8192),
        m_activeGain(nullptr),
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
        m_eqHighGain(0.0f)
    {}

    AudioStream::~AudioStream()
    {
        Close();
    }

    // Live Setters
    void AudioStream::SetGain(float gain) { m_gainValue = gain; if (m_activeGain) m_activeGain->SetGain(gain); }
    void AudioStream::SetDelayFeedback(float feedback) { m_delayFeedbackValue = feedback; if (m_activeDelay) m_activeDelay->SetFeedback(feedback); }
    void AudioStream::SetLowPassAlpha(float alpha) { m_lowPassAlphaValue = alpha; }
    void AudioStream::SetHighPassAlpha(float alpha) { m_highPassAlphaValue = alpha; }
    void AudioStream::SetDelaySamples(int samples) { m_delaySamplesValue = samples; }

    void AudioStream::SetEqLowGain(float dbGain) { m_eqLowGain = dbGain; if (m_activeEqualizer) m_activeEqualizer->SetLowGain(dbGain); }
    void AudioStream::SetEqMidGain(float dbGain) { m_eqMidGain = dbGain; if (m_activeEqualizer) m_activeEqualizer->SetMidGain(dbGain); }
    void AudioStream::SetEqHighGain(float dbGain) { m_eqHighGain = dbGain; if (m_activeEqualizer) m_activeEqualizer->SetHighGain(dbGain); }

    void AudioStream::EnableGain(bool enable) { m_gainEnabled = enable; }
    void AudioStream::EnableLowPass(bool enable) { m_lowPassEnabled = enable; }
    void AudioStream::EnableHighPass(bool enable) { m_highPassEnabled = enable; }
    void AudioStream::EnableDelay(bool enable) { m_delayEnabled = enable; }
    void AudioStream::EnableEqualizer(bool enable) { m_equalizerEnabled = enable; }

    bool AudioStream::OpenDefaultStream()
    {
        Logger::Info("Opening default audio stream...");

        if (m_gainEnabled)
        {
            auto gain = std::make_unique<Gain>();
            m_activeGain = gain.get();
            m_activeGain->SetGain(m_gainValue);
            m_dspChain.AddModule(std::move(gain));
        }

        if (m_lowPassEnabled)
        {
            auto lowPass = std::make_unique<LowPassFilter>();
            lowPass->SetAlpha(m_lowPassAlphaValue);
            m_dspChain.AddModule(std::move(lowPass));
        }

        if (m_highPassEnabled)
        {
            auto highPass = std::make_unique<HighPassFilter>();
            highPass->SetAlpha(m_highPassAlphaValue);
            m_dspChain.AddModule(std::move(highPass));
        }

        if (m_equalizerEnabled)
        {
            auto eq = std::make_unique<Equalizer>();
            m_activeEqualizer = eq.get();
            m_activeEqualizer->SetLowGain(m_eqLowGain);
            m_activeEqualizer->SetMidGain(m_eqMidGain);
            m_activeEqualizer->SetHighGain(m_eqHighGain);
            m_dspChain.AddModule(std::move(eq));
        }

        if (m_delayEnabled)
        {
            auto delay = std::make_unique<Delay>();
            m_activeDelay = delay.get();
            m_activeDelay->SetDelaySamples(m_delaySamplesValue);
            m_activeDelay->SetFeedback(m_delayFeedbackValue);
            m_dspChain.AddModule(std::move(delay));
        }

        PaError error = Pa_OpenDefaultStream(&m_stream, 1, 1, paFloat32, 48000, 512, AudioCallback, this);

        if (error != paNoError) return false;

        Logger::Info("Audio stream opened successfully.");
        return true;
    }

    bool AudioStream::Start()
    {
        if (!m_stream) return false;
        return Pa_StartStream(m_stream) == paNoError;
    }

    void AudioStream::Stop()
    {
        if (m_stream) Pa_StopStream(m_stream);
    }

    void AudioStream::Close()
    {
        if (m_stream)
        {
            Pa_CloseStream(m_stream);
            m_stream = nullptr;
        }
    }

    int AudioStream::ProcessAudio(const void* input, void* output, unsigned long frameCount)
    {
        const float* inputBuffer = static_cast<const float*>(input);
        float* outputBuffer = static_cast<float*>(output);

        if (!inputBuffer)
        {
            for (unsigned long i = 0; i < frameCount; ++i) outputBuffer[i] = 0.0f;
            return paContinue;
        }

        // TYPO FIXED HERE: frameCount
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
        return paContinue;
    }

} // namespace AudioEngine