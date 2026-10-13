#pragma once

#include <string>

namespace AudioEngine
{
    class ConfigManager
    {
    public:

        bool Load(const std::string& filePath);

        int GetSampleRate() const;

        int GetBufferSize() const;

        int GetChannels() const;

        bool IsDebugEnabled() const;

        float GetGain() const;

        float GetLowPassAlpha() const;

        float GetHighPassAlpha() const;

        int GetDelaySamples() const;

        float GetDelayFeedback() const;

        bool IsGainEnabled() const;

        bool IsLowPassEnabled() const;

        bool IsHighPassEnabled() const;

        bool IsDelayEnabled() const;

    private:

        int m_sampleRate;

        int m_bufferSize;

        int m_channels;

        bool m_enableDebug;

        float m_gain;

        float m_lowPassAlpha;

        float m_highPassAlpha;

        int m_delaySamples;

        float m_delayFeedback;

        bool m_gainEnabled;

        bool m_lowPassEnabled;

        bool m_highPassEnabled;

        bool m_delayEnabled;
    };
}
