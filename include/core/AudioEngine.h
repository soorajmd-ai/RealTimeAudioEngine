#pragma once

#include "ConfigManager.h"
#include "thread/ThreadManager.h"

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

    private:

        ConfigManager m_configManager;

        ThreadManager m_threadManager;
    };
}
