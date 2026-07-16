#include "Logger.h"
#include "ConfigManager.h"
#include "buffer/RingBuffer.h"

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

    AudioEngine::RingBuffer<int> buffer(5);

    buffer.push(10);
    buffer.push(20);
    buffer.push(30);
    buffer.push(40);
    buffer.push(50);

    int value;

    // Pop first 3 elements
    for (int i = 0; i < 3; i++)
    {
        buffer.pop(value);
        std::cout << "Popped: " << value << std::endl;
    }

    // Push again
    buffer.push(60);
    buffer.push(70);
    buffer.push(80);

    // Pop everything
    while (buffer.pop(value))
    {
        std::cout << "Popped: " << value << std::endl;
    }

    return 0;
}