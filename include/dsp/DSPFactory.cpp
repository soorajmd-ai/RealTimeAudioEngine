#include "dsp/DSPFactory.h"

#include "dsp/DSPModule.h"
#include "dsp/Gain.h"
#include "dsp/LowPassFilter.h"
#include "dsp/HighPassFilter.h"
#include "dsp/Delay.h"

namespace AudioEngine
{

    DSPModule* DSPFactory::Create(
        const std::string& moduleName)
    {
        if (moduleName == "Gain")
        {
            return new Gain();
        }

        if (moduleName == "LowPass")
        {
            return new LowPassFilter();
        }

        if (moduleName == "HighPass")
        {
            return new HighPassFilter();
        }

        if (moduleName == "Delay")
        {
            return new Delay();
        }

        return nullptr;
    }

}