#pragma once

#include <string>
#include <memory>

namespace AudioEngine
{
    class DSPModule;

    class DSPFactory
    {
    public:
        static std::unique_ptr<DSPModule> Create(const std::string& moduleName);
    };
}