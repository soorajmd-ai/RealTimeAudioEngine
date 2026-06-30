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
}