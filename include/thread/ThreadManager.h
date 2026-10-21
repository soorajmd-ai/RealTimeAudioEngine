#pragma once

#include <thread>
#include <atomic>

namespace AudioEngine
{
    class ThreadManager
    {
    public:

        ThreadManager();

        ~ThreadManager();

        void Start();

        void Stop();

        bool IsRunning() const;

    private:

        void Worker();

        std::thread m_thread;

        std::atomic<bool> m_running;
    };
}