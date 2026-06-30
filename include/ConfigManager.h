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

    private:

        int m_sampleRate;

        int m_bufferSize;

        int m_channels;

        bool m_enableDebug;
    };
}
