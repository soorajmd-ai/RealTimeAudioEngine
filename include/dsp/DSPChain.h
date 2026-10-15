#pragma once

#include <vector>
#include <memory>
#include "dsp/DSPModule.h"

namespace AudioEngine
{
    class DSPChain
    {
    public:
        void AddModule(std::unique_ptr<DSPModule> module);

        float Process(float sample);

    private:
        std::vector<std::unique_ptr<DSPModule>> m_modules;
    };
}