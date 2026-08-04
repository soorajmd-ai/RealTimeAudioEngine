#include "io/AudioStream.h"
#include "Logger.h"
#include "buffer/RingBuffer.h"
#include <atomic>

static std::atomic<int> g_callbackCount = 0;
static AudioEngine::RingBuffer<float> g_audioBuffer(8192);

static int AudioCallback(
    const void* input,
    void* output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo*,
    PaStreamCallbackFlags,
    void*)
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
        g_audioBuffer.push(inputBuffer[i]);
    }

    // Playback from our RingBuffer
    for (unsigned long i = 0; i < frameCount; ++i)
    {
        float sample = 0.0f;

        if (g_audioBuffer.pop(sample))
        {
            outputBuffer[i] = sample;
        }
        else
        {
            outputBuffer[i] = 0.0f;
        }
    }

    return paContinue;
}

namespace AudioEngine
{

    AudioStream::AudioStream()
        :
        m_stream(nullptr)
    {}

    AudioStream::~AudioStream()
    {}

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
                nullptr); 

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