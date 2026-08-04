#include "io/AudioStream.h"
#include "Logger.h"

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

        return true;
    }

    bool AudioStream::Start()
    {
        Logger::Info("Starting audio stream...");

        return true;
    }

    void AudioStream::Stop()
    {
        Logger::Info("Stopping audio stream...");
    }

    void AudioStream::Close()
    {
        Logger::Info("Closing audio stream...");
    }

}