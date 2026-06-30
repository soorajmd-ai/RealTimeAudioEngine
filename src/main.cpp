#include "Logger.h"
#include "ConfigManager.h"

#include <iostream>
#include <filesystem>

int main()
{
    AudioEngine::Logger::Info("Real-Time Audio Processing Engine");
    AudioEngine::Logger::Info("Version 0.1.0");

    std::cout << "Current Working Directory:\n";
    std::cout << std::filesystem::current_path() << std::endl;

    AudioEngine::ConfigManager config;

    if (!config.Load("config/audio_config.json"))
    {
        AudioEngine::Logger::Error("Failed to load configuration.");
        return -1;
    }

    AudioEngine::Logger::Info("Configuration loaded successfully.");

    std::cout << "\n===== Configuration =====\n";
    std::cout << "Sample Rate : " << config.GetSampleRate() << '\n';
    std::cout << "Buffer Size : " << config.GetBufferSize() << '\n';
    std::cout << "Channels    : " << config.GetChannels() << '\n';
    std::cout << "Debug Mode  : "
        << std::boolalpha
        << config.IsDebugEnabled()
        << '\n';

    return 0;
}