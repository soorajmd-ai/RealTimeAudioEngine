#include "dsp/DSPFactory.h"
#include "dsp/DSPModule.h"
#include "dsp/Gain.h"
#include "dsp/LowPassFilter.h"
#include "dsp/HighPassFilter.h"
#include "dsp/Delay.h"

namespace AudioEngine
{

    std::unique_ptr<DSPModule> DSPFactory::Create(const std::string& moduleName)
    {
        if (moduleName == "Gain")
        {
            return std::make_unique<Gain>();
        }

        if (moduleName == "LowPass")
        {
            return std::make_unique<LowPassFilter>();
        }

        if (moduleName == "HighPass")
        {
            return std::make_unique<HighPassFilter>();
        }

        if (moduleName == "Delay")
        {
            return std::make_unique<Delay>();
        }

        return nullptr;
    }

}