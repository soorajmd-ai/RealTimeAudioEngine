#pragma once

#include <string>

namespace AudioEngine
{
    class DSPModule;

    class DSPFactory
    {
    public:

        static DSPModule* Create(
            const std::string& moduleName);
    };
}
