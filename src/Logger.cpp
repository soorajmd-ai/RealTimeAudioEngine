#include "Logger.h"

#include <iostream>

namespace AudioEngine
{
    void Logger::Info(const std::string& message)
    {
        std::cout << "[INFO] " << message << std::endl;
    }

    void Logger::Warning(const std::string& message)
    {
        std::cout << "[WARNING] " << message << std::endl;
    }

    void Logger::Error(const std::string& message)
    {
        std::cout << "[ERROR] " << message << std::endl;
    }

    void Logger::Debug(const std::string& message)
    {
        std::cout << "[DEBUG] " << message << std::endl;
    }
}