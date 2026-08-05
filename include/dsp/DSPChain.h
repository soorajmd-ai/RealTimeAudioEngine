#pragma once

#include <vector>

#include "dsp/DSPModule.h"

namespace AudioEngine
{
    class DSPChain
    {
    public:

        void AddModule(DSPModule* module);

        float Process(float sample);

    private:

        std::vector<DSPModule*> m_modules;
    };
}
