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

        m_audioStream.SetGain(
            m_configManager.GetGain());

        m_audioStream.SetLowPassAlpha(
            m_configManager.GetLowPassAlpha());

        if (!m_portAudio.Initialize())
        {
            return false;
        }

        if (!m_audioStream.OpenDefaultStream())
        {
            return false;
        }

        return true;
    }

    void AudioEngine::Run()
    {
        Logger::Info("Starting Audio Engine...");

        m_audioStream.Start();

        m_threadManager.Start();
    }

    void AudioEngine::Shutdown()
    {
        Logger::Info("Shutting down Audio Engine...");

        m_audioStream.Stop();

        m_audioStream.Close();

        m_portAudio.Terminate();

        m_threadManager.Stop();
    }

}