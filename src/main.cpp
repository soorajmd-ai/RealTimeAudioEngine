#include "Logger.h"
#include "ConfigManager.h"
#include "buffer/RingBuffer.h"
#include "thread/ThreadManager.h"
#include "core/AudioEngine.h"

#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>

int main()
{
    AudioEngine::AudioEngine engine;

    if (!engine.Initialize())
    {
        return -1;
    }

    engine.Run();

    std::this_thread::sleep_for(std::chrono::seconds(5));

    engine.Shutdown();

    return 0;
}