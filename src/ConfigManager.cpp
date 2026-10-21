#include "ConfigManager.h"

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

namespace AudioEngine
{
    bool ConfigManager::Load(const std::string& filePath)
    {
        std::ifstream file(filePath);

        if (!file.is_open())
        {
            std::cout << "[ERROR] Failed to open configuration file.\n";
            return false;
        }

        nlohmann::json jsonData;

        file >> jsonData;

        m_sampleRate = jsonData["sampleRate"];
        m_bufferSize = jsonData["bufferSize"];
        m_channels = jsonData["channels"];
        m_enableDebug = jsonData["enableDebug"];

        m_gainEnabled = jsonData["gainEnabled"];

        m_lowPassEnabled = jsonData["lowPassEnabled"];

        m_highPassEnabled = jsonData["highPassEnabled"];

        m_delayEnabled = jsonData["delayEnabled"];

        m_gain = jsonData["gain"];

        m_lowPassAlpha = jsonData["lowPassAlpha"];

        m_highPassAlpha = jsonData["highPassAlpha"];

        m_delaySamples = jsonData["delaySamples"];

        m_delayFeedback = jsonData["delayFeedback"];

        return true;
    }

    int ConfigManager::GetSampleRate() const
    {
        return m_sampleRate;
    }

    int ConfigManager::GetBufferSize() const
    {
        return m_bufferSize;
    }

    int ConfigManager::GetChannels() const
    {
        return m_channels;
    }

    bool ConfigManager::IsDebugEnabled() const
    {
        return m_enableDebug;
    }

    float ConfigManager::GetGain() const
    {
        return m_gain;
    }

    float ConfigManager::GetLowPassAlpha() const
    {
        return m_lowPassAlpha;
    }

    float ConfigManager::GetHighPassAlpha() const
    {
        return m_highPassAlpha;
    }

    int ConfigManager::GetDelaySamples() const
    {
        return m_delaySamples;
    }

    float ConfigManager::GetDelayFeedback() const
    {
        return m_delayFeedback;
    }

    bool ConfigManager::IsGainEnabled() const
    {
        return m_gainEnabled;
    }

    bool ConfigManager::IsLowPassEnabled() const
    {
        return m_lowPassEnabled;
    }

    bool ConfigManager::IsHighPassEnabled() const
    {
        return m_highPassEnabled;
    }

    bool ConfigManager::IsDelayEnabled() const
    {
        return m_delayEnabled;
    }
}