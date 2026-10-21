#pragma once

#include <string>

namespace AudioEngine
{
    class Logger
    {
    public:

        static void Info(const std::string& message);

        static void Warning(const std::string& message);

        static void Error(const std::string& message);

        static void Debug(const std::string& message);
    };
}