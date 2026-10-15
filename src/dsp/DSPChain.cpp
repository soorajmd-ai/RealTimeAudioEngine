#include "dsp/DSPChain.h"
#include <utility>

namespace AudioEngine
{

    void DSPChain::AddModule(std::unique_ptr<DSPModule> module)
    {
        m_modules.push_back(std::move(module));
    }

    float DSPChain::Process(float sample)
    {
        for (auto& module : m_modules)
        {
            sample = module->Process(sample);
        }

        return sample;
    }

}