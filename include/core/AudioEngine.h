#pragma once

#include "ConfigManager.h"
#include "thread/ThreadManager.h"
#include "io/PortAudioWrapper.h"
#include "io/AudioStream.h"

namespace AudioEngine
{
    class AudioEngine
    {
    public:
        AudioEngine();
        ~AudioEngine();

        bool Initialize();
        void Run();
        void Shutdown();

        // Live GUI Controls
        void SetLiveGain(float gain);
        void SetLiveDelayFeedback(float feedback);
        void SetLiveLowPassAlpha(float alpha);
        void SetLiveHighPassAlpha(float alpha);
        void SetLiveEqLow(float dbGain);
        void SetLiveEqMid(float dbGain);
        void SetLiveEqHigh(float dbGain);

        float GetLiveGain() const;
        float GetLiveDelayFeedback() const;
        float GetLiveLowPassAlpha() const;
        float GetLiveHighPassAlpha() const;
        float GetLiveEqLow() const;
        float GetLiveEqMid() const;
        float GetLiveEqHigh() const;

    private:
        ConfigManager m_configManager;
        ThreadManager m_threadManager;
        PortAudioWrapper m_portAudio;
        AudioStream m_audioStream;
    };
}