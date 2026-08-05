#include "dsp/DSPChain.h"

namespace AudioEngine
{

    void DSPChain::AddModule(DSPModule* module)
    {
        m_modules.push_back(module);
    }

    float DSPChain::Process(float sample)
    {
        for (DSPModule* module : m_modules)
        {
            sample = module->Process(sample);
        }

        return sample;
    }

}