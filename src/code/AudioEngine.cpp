#include "core/AudioEngine.h"
#include "Logger.h"
#include "dsp/DSPFactory.h"

namespace AudioEngine
{
    AudioEngine::AudioEngine()
        : m_configManager(),
        m_threadManager(),
        m_portAudio(),
        m_audioStream()
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
            Logger::Error("Failed to load audio configuration.");
            Shutdown();
            return false;
        }

        m_audioStream.EnableGain(
            m_configManager.IsGainEnabled());

        m_audioStream.EnableLowPass(
            m_configManager.IsLowPassEnabled());

        m_audioStream.EnableHighPass(
            m_configManager.IsHighPassEnabled());

        m_audioStream.EnableDelay(
            m_configManager.IsDelayEnabled());

        m_audioStream.SetGain(
            m_configManager.GetGain());

        m_audioStream.SetLowPassAlpha(
            m_configManager.GetLowPassAlpha());

        m_audioStream.SetHighPassAlpha(
            m_configManager.GetHighPassAlpha());

        m_audioStream.SetDelaySamples(
            m_configManager.GetDelaySamples());

        m_audioStream.SetDelayFeedback(
            m_configManager.GetDelayFeedback());

        if (!m_portAudio.Initialize())
        {
            Logger::Error("PortAudio initialization failed.");
            Shutdown();
            return false;
        }

        if (!m_audioStream.OpenDefaultStream())
        {
            Logger::Error("Failed to open the default audio stream.");
            Shutdown();
            return false;
        }

        return true;
    }

    bool AudioEngine::Run()
    {
        Logger::Info("Starting Audio Engine...");

        if (!m_audioStream.Start())
        {
            Logger::Error("Failed to start the audio stream.");
            return false;
        }

        try
        {
            m_threadManager.Start();
        }
        catch (...)
        {
            // Reset worker-thread state if thread creation fails.
            m_threadManager.Stop();

            // The audio stream has already started.
            m_audioStream.Stop();

            Logger::Error("Failed to start the engine worker thread.");
            return false;
        }

        return true;
    }

    void AudioEngine::Shutdown()
    {
        // Stop background work before releasing audio resources.
        m_threadManager.Stop();

        // Stop and close the stream before terminating PortAudio.
        m_audioStream.Stop();
        m_audioStream.Close();

        m_portAudio.Terminate();

        Logger::Info("Audio Engine shutdown completed.");
    }

    // ---------------------------------------------------------
    // Live parameter setters
    // ---------------------------------------------------------

    void AudioEngine::SetLiveGain(float gain)
    {
        m_audioStream.SetGain(gain);
    }

    void AudioEngine::SetLiveDelayFeedback(float feedback)
    {
        m_audioStream.SetDelayFeedback(feedback);
    }

    void AudioEngine::SetLiveLowPassAlpha(float alpha)
    {
        m_audioStream.SetLowPassAlpha(alpha);
    }

    void AudioEngine::SetLiveHighPassAlpha(float alpha)
    {
        m_audioStream.SetHighPassAlpha(alpha);
    }

    void AudioEngine::SetLiveEqLow(float dbGain)
    {
        m_audioStream.SetEqLowGain(dbGain);
    }

    void AudioEngine::SetLiveEqMid(float dbGain)
    {
        m_audioStream.SetEqMidGain(dbGain);
    }

    void AudioEngine::SetLiveEqHigh(float dbGain)
    {
        m_audioStream.SetEqHighGain(dbGain);
    }

    // ---------------------------------------------------------
    // Live parameter getters
    // ---------------------------------------------------------

    float AudioEngine::GetLiveGain() const
    {
        return m_audioStream.GetGain();
    }

    float AudioEngine::GetLiveDelayFeedback() const
    {
        return m_audioStream.GetDelayFeedback();
    }

    float AudioEngine::GetLiveLowPassAlpha() const
    {
        return m_audioStream.GetLowPassAlpha();
    }

    float AudioEngine::GetLiveHighPassAlpha() const
    {
        return m_audioStream.GetHighPassAlpha();
    }

    float AudioEngine::GetLiveEqLow() const
    {
        return m_audioStream.GetEqLowGain();
    }

    float AudioEngine::GetLiveEqMid() const
    {
        return m_audioStream.GetEqMidGain();
    }

    float AudioEngine::GetLiveEqHigh() const
    {
        return m_audioStream.GetEqHighGain();
    }

    // ---------------------------------------------------------
    // Visualizer data
    // ---------------------------------------------------------

    const std::vector<float>& AudioEngine::GetVisualizerData()
    {
        return m_audioStream.GetVisualizerData();
    }

    // ---------------------------------------------------------
    // Dynamic graph wiring
    // ---------------------------------------------------------

    void AudioEngine::SetGraphLinks(
        const std::vector<std::pair<int, int>>& links)
    {
        m_audioStream.SetGraphLinks(links);
    }

    bool AudioEngine::AddDSPModule(
        int nodeId,
        const std::string& moduleName)
    {
        return m_audioStream.AddDSPModule(nodeId, moduleName);
    }

} // namespace AudioEngine
