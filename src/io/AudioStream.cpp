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

    // PortAudio C-style callback
    static int AudioCallback(
        const void* input,
        void* output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData)
    {
        AudioStream* audioStream = static_cast<AudioStream*>(userData);
        return audioStream->ProcessAudio(input, output, frameCount);
    }

    AudioStream::AudioStream()
        :
        m_stream(nullptr),
        m_ringBuffer(8192),
        m_gainEnabled(true),
        m_lowPassEnabled(true),
        m_highPassEnabled(true),
        m_delayEnabled(true),
        m_equalizerEnabled(false),
        m_gainValue(1.0f),
        m_lowPassAlphaValue(0.2f),
        m_highPassAlphaValue(0.95f),
        m_delaySamplesValue(12000),
        m_delayFeedbackValue(0.5f)
    {}

    AudioStream::~AudioStream()
    {
        Close();
    }

    void AudioStream::SetGain(float gain) { m_gainValue = gain; }
    void AudioStream::SetLowPassAlpha(float alpha) { m_lowPassAlphaValue = alpha; }
    void AudioStream::SetHighPassAlpha(float alpha) { m_highPassAlphaValue = alpha; }
    void AudioStream::SetDelaySamples(int samples) { m_delaySamplesValue = samples; }
    void AudioStream::SetDelayFeedback(float feedback) { m_delayFeedbackValue = feedback; }

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
            Logger::Info("Adding Gain Module");
            auto gain = std::make_unique<Gain>();
            gain->SetGain(m_gainValue);
            m_dspChain.AddModule(std::move(gain));
        }

        if (m_lowPassEnabled)
        {
            Logger::Info("Adding LowPass Module");
            auto lowPass = std::make_unique<LowPassFilter>();
            lowPass->SetAlpha(m_lowPassAlphaValue);
            m_dspChain.AddModule(std::move(lowPass));
        }

        if (m_highPassEnabled)
        {
            Logger::Info("Adding HighPass Module");
            auto highPass = std::make_unique<HighPassFilter>();
            highPass->SetAlpha(m_highPassAlphaValue);
            m_dspChain.AddModule(std::move(highPass));
        }

        if (m_delayEnabled)
        {
            Logger::Info("Adding Delay Module");
            auto delay = std::make_unique<Delay>();
            delay->SetDelaySamples(m_delaySamplesValue);
            delay->SetFeedback(m_delayFeedbackValue);
            m_dspChain.AddModule(std::move(delay));
        }

        if (m_equalizerEnabled)
        {
            Logger::Info("Adding Equalizer Module");
            auto equalizer = std::make_unique<Equalizer>();
            m_dspChain.AddModule(std::move(equalizer));
        }

        PaError error = Pa_OpenDefaultStream(
            &m_stream,
            1, // input channels
            1, // output channels
            paFloat32,
            48000,
            512,
            AudioCallback,
            this); // Pass 'this' to the callback

        if (error != paNoError)
        {
            Logger::Error(Pa_GetErrorText(error));
            return false;
        }

        Logger::Info("Audio stream opened successfully.");
        return true;
    }

    bool AudioStream::Start()
    {
        Logger::Info("Starting audio stream...");
        if (!m_stream) return false;

        PaError error = Pa_StartStream(m_stream);
        if (error != paNoError)
        {
            Logger::Error(Pa_GetErrorText(error));
            return false;
        }
        return true;
    }

    void AudioStream::Stop()
    {
        Logger::Info("Stopping audio stream...");
        if (m_stream)
        {
            Pa_StopStream(m_stream);
        }
    }

    void AudioStream::Close()
    {
        Logger::Info("Closing audio stream...");
        if (m_stream)
        {
            Pa_CloseStream(m_stream);
            m_stream = nullptr;
        }
    }

    int AudioStream::ProcessAudio(
        const void* input,
        void* output,
        unsigned long frameCount)
    {
        const float* inputBuffer = static_cast<const float*>(input);
        float* outputBuffer = static_cast<float*>(output);

        if (!inputBuffer)
        {
            for (unsigned long i = 0; i < frameCount; ++i)
            {
                outputBuffer[i] = 0.0f;
            }
            return paContinue;
        }

        // Capture microphone samples
        for (unsigned long i = 0; i < frameCount; ++i)
        {
            m_ringBuffer.push(inputBuffer[i]);
        }

        // Playback through DSP Chain
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