#include "Logger.h"

int main()
{
    AudioEngine::Logger::Info("Real-Time Audio Processing Engine");
    AudioEngine::Logger::Info("Version 0.1.0");

    AudioEngine::Logger::Warning("This is a warning message.");

    AudioEngine::Logger::Error("This is an error message.");

    AudioEngine::Logger::Debug("Debug logging enabled.");

    return 0;
}