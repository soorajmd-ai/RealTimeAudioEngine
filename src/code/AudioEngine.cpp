#include "core/AudioEngine.h"
#include "Logger.h"

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
            Logger::Error("Failed to load configuration.");
            return false;
        }

        Logger::Info("Configuration loaded successfully.");

        m_audioStream.EnableGain(m_configManager.IsGainEnabled());
        m_audioStream.EnableLowPass(m_configManager.IsLowPassEnabled());
        m_audioStream.EnableHighPass(m_configManager.IsHighPassEnabled());
        m_audioStream.EnableDelay(m_configManager.IsDelayEnabled());

        m_audioStream.SetGain(m_configManager.GetGain());
        m_audioStream.SetLowPassAlpha(m_configManager.GetLowPassAlpha());
        m_audioStream.SetHighPassAlpha(m_configManager.GetHighPassAlpha());
        m_audioStream.SetDelaySamples(m_configManager.GetDelaySamples());
        m_audioStream.SetDelayFeedback(m_configManager.GetDelayFeedback());

        if (!m_portAudio.Initialize()) return false;
        if (!m_audioStream.OpenDefaultStream()) return false;

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

    // --- LIVE GUI CONTROLS ---

    void AudioEngine::SetLiveGain(float gain) { m_audioStream.SetGain(gain); }
    void AudioEngine::SetLiveDelayFeedback(float feedback) { m_audioStream.SetDelayFeedback(feedback); }
    void AudioEngine::SetLiveLowPassAlpha(float alpha) { m_audioStream.SetLowPassAlpha(alpha); }
    void AudioEngine::SetLiveHighPassAlpha(float alpha) { m_audioStream.SetHighPassAlpha(alpha); }

    void AudioEngine::SetLiveEqLow(float dbGain) { m_audioStream.SetEqLowGain(dbGain); }
    void AudioEngine::SetLiveEqMid(float dbGain) { m_audioStream.SetEqMidGain(dbGain); }
    void AudioEngine::SetLiveEqHigh(float dbGain) { m_audioStream.SetEqHighGain(dbGain); }

    float AudioEngine::GetLiveGain() const { return m_audioStream.GetGain(); }
    float AudioEngine::GetLiveDelayFeedback() const { return m_audioStream.GetDelayFeedback(); }
    float AudioEngine::GetLiveLowPassAlpha() const { return m_configManager.GetLowPassAlpha(); }
    float AudioEngine::GetLiveHighPassAlpha() const { return m_configManager.GetHighPassAlpha(); }

    float AudioEngine::GetLiveEqLow() const { return m_audioStream.GetEqLowGain(); }
    float AudioEngine::GetLiveEqMid() const { return m_audioStream.GetEqMidGain(); }
    float AudioEngine::GetLiveEqHigh() const { return m_audioStream.GetEqHighGain(); }

    // --- VISUALIZER ---
    const std::vector<float>& AudioEngine::GetVisualizerData() const
    {
        return m_audioStream.GetVisualizerData();
    }

} // namespace AudioEngine