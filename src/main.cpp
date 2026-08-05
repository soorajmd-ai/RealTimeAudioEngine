#include "core/AudioEngine.h"
#include "Logger.h"

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