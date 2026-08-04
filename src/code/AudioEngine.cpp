#include "core/AudioEngine.h"
#include "Logger.h"

namespace AudioEngine
{

    AudioEngine::AudioEngine()
    {}

    AudioEngine::~AudioEngine()
    {
        Shutdown();
    }

    bool AudioEngine::Initialize()
    {
        Logger::Info("Initializing Audio Engine...");

        if (!m_configManager.Load("config/audio_config.json"))
        {
            Logger::Error("Failed to load configuration.");

            return false;
        }

        Logger::Info("Configuration loaded successfully.");

        return true;
    }

    void AudioEngine::Run()
    {
        Logger::Info("Starting Audio Engine...");

        m_threadManager.Start();
    }

    void AudioEngine::Shutdown()
    {
        Logger::Info("Shutting down Audio Engine...");

        m_threadManager.Stop();
    }

}