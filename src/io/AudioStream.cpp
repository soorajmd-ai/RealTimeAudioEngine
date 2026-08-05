#include "io/AudioStream.h"
#include "Logger.h"
#include "buffer/RingBuffer.h"
#include "dsp/Gain.h"
#include "dsp/DSPChain.h"
#include "dsp/LowPassFilter.h"
#include <atomic>

static std::atomic<int> g_callbackCount = 0;

static int AudioCallback(
    const void* input,
    void* output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo*,
    PaStreamCallbackFlags,
    void* userData)
{
    AudioEngine::AudioStream* audioStream =
        static_cast<AudioEngine::AudioStream*>(userData);

    return audioStream->ProcessAudio(
        input,
        output,
        frameCount);
}

namespace AudioEngine
{

    AudioStream::AudioStream()
        :
        m_stream(nullptr),
        m_ringBuffer(8192)
    {}

    AudioStream::~AudioStream()
    {}

    void AudioStream::SetGain(float gain)
    {
        m_gain.SetGain(gain);
    }

    void AudioStream::SetLowPassAlpha(float alpha)
    {
        m_lowPass.SetAlpha(alpha);
    }

    int AudioEngine::AudioStream::ProcessAudio(
        const void* input,
        void* output,
        unsigned long frameCount)
    {
        const float* inputBuffer =
            static_cast<const float*>(input);

        float* outputBuffer =
            static_cast<float*>(output);

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

        // Playback from our RingBuffer
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

    bool AudioStream::OpenDefaultStream()
    {
        Logger::Info("Opening default audio stream...");

        PaError error =
            Pa_OpenDefaultStream(
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
            Logger::Error(Pa_GetErrorText(error));
            return false;
        }

        Logger::Info("Audio stream opened successfully.");

        m_delay.SetDelaySamples(12000);

        m_delay.SetFeedback(0.6f);

        m_dspChain.AddModule(&m_gain);

        m_dspChain.AddModule(&m_lowPass);

        m_dspChain.AddModule(&m_delay);

        return true;
    }

    bool AudioStream::Start()
    {
        Logger::Info("Starting audio stream...");

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

        Logger::Info(
            "Audio Callback Count: " +
            std::to_string(g_callbackCount.load()));

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

}