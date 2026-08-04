#include "thread/ThreadManager.h"
#include <chrono>
#include <iostream>

namespace AudioEngine
{

    ThreadManager::ThreadManager()
        :
        m_running(false)
    {}

    ThreadManager::~ThreadManager()
    {
        Stop();
    }

    void ThreadManager::Start()
    {
        if (m_running)
        {
            return;
        }

        m_running = true;

        m_thread = std::thread(&ThreadManager::Worker, this);
    }

    void ThreadManager::Stop()
    {
        m_running = false;

        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }

    void ThreadManager::Worker()
    {
        while (m_running)
        {
            std::cout << "Worker Thread Running..." << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    bool ThreadManager::IsRunning() const
    {
        return m_running;
    }

}