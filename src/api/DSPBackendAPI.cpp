
#include "api/DSPBackendAPI.h"
#include "core/AudioEngine.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>

struct DSPBackendInstance
{
    AudioEngine::AudioEngine engine;
    bool initialized = false;
    bool running = false;
    std::string lastError;
};

namespace
{
    DSPBackendInstance* CreateInstance()
    {
        try
        {
            return new (std::nothrow) DSPBackendInstance();
        }
        catch (...)
        {
            return nullptr;
        }
    }

    void DestroyInstance(DSPBackendInstance* instance)
    {
        if (instance == nullptr)
        {
            return;
        }

        try
        {
            delete instance;
        }
        catch (...)
        {
            // Prevent C++ exceptions from escaping the C API.
        }
    }

    DSPBackendResult InitializeInstance(DSPBackendInstance* instance)
    {
        if (instance == nullptr)
        {
            return DSP_BACKEND_INVALID_ARGUMENT;
        }

        if (instance->initialized)
        {
            return DSP_BACKEND_OK;
        }

        try
        {
            if (!instance->engine.Initialize())
            {
                instance->lastError = "AudioEngine initialization failed.";
                return DSP_BACKEND_ERROR;
            }

            instance->initialized = true;
            instance->lastError.clear();
            return DSP_BACKEND_OK;
        }
        catch (...)
        {
            instance->lastError = "Unexpected exception during initialization.";
            return DSP_BACKEND_ERROR;
        }
    }

    DSPBackendResult StartInstance(DSPBackendInstance* instance)
    {
        if (instance == nullptr)
        {
            return DSP_BACKEND_INVALID_ARGUMENT;
        }

        if (!instance->initialized)
        {
            instance->lastError = "Initialize the backend before starting it.";
            return DSP_BACKEND_NOT_INITIALIZED;
        }

        if (instance->running)
        {
            return DSP_BACKEND_OK;
        }


        try
        {
            if (!instance->engine.Run())
            {
                instance->lastError =
                    "AudioEngine failed to start the audio stream.";
                return DSP_BACKEND_ERROR;
            }

            instance->running = true;
            instance->lastError.clear();
            return DSP_BACKEND_OK;
        }
        catch (...)
        {
            instance->engine.Shutdown();
            instance->running = false;
            instance->initialized = false;

            instance->lastError =
                "Unexpected exception while starting the engine.";

            return DSP_BACKEND_ERROR;
        }

    }

    void StopInstance(DSPBackendInstance* instance)
    {
        if (instance == nullptr || !instance->initialized)
        {
            return;
        }

        try
        {
            // The current AudioEngine::Shutdown() stops and closes
            // the stream and terminates PortAudio. This is not a
            // pause/resume operation; restarting requires reinitialization.
            instance->engine.Shutdown();
            instance->initialized = false;
            instance->running = false;
        }
        catch (...)
        {
            instance->lastError = "Unexpected exception while shutting down the engine.";
        }
    }

    DSPBackendResult SetParameter(
        DSPBackendInstance* instance,
        uint32_t parameterId,
        float value)
    {
        if (instance == nullptr)
        {
            return DSP_BACKEND_INVALID_ARGUMENT;
        }

        if (!instance->initialized)
        {
            instance->lastError = "Initialize the backend before changing parameters.";
            return DSP_BACKEND_NOT_INITIALIZED;
        }

        try
        {
            switch (parameterId)
            {
            case DSP_PARAMETER_GAIN:
                instance->engine.SetLiveGain(value);
                break;

            case DSP_PARAMETER_DELAY_FEEDBACK:
                instance->engine.SetLiveDelayFeedback(value);
                break;

            case DSP_PARAMETER_LOW_PASS_ALPHA:
                instance->engine.SetLiveLowPassAlpha(value);
                break;

            case DSP_PARAMETER_HIGH_PASS_ALPHA:
                instance->engine.SetLiveHighPassAlpha(value);
                break;

            case DSP_PARAMETER_EQ_LOW:
                instance->engine.SetLiveEqLow(value);
                break;

            case DSP_PARAMETER_EQ_MID:
                instance->engine.SetLiveEqMid(value);
                break;

            case DSP_PARAMETER_EQ_HIGH:
                instance->engine.SetLiveEqHigh(value);
                break;

            default:
                instance->lastError = "Unknown DSP parameter ID.";
                return DSP_BACKEND_INVALID_ARGUMENT;
            }

            instance->lastError.clear();
            return DSP_BACKEND_OK;
        }
        catch (...)
        {
            instance->lastError = "Unexpected exception while setting a parameter.";
            return DSP_BACKEND_ERROR;
        }
    }

    DSPBackendResult GetParameter(
        DSPBackendInstance* instance,
        uint32_t parameterId,
        float* valueOut)
    {
        if (instance == nullptr || valueOut == nullptr)
        {
            return DSP_BACKEND_INVALID_ARGUMENT;
        }

        if (!instance->initialized)
        {
            instance->lastError = "Initialize the backend before reading parameters.";
            return DSP_BACKEND_NOT_INITIALIZED;
        }

        try
        {
            switch (parameterId)
            {
            case DSP_PARAMETER_GAIN:
                *valueOut = instance->engine.GetLiveGain();
                break;

            case DSP_PARAMETER_DELAY_FEEDBACK:
                *valueOut = instance->engine.GetLiveDelayFeedback();
                break;

            case DSP_PARAMETER_LOW_PASS_ALPHA:
                *valueOut = instance->engine.GetLiveLowPassAlpha();
                break;

            case DSP_PARAMETER_HIGH_PASS_ALPHA:
                *valueOut = instance->engine.GetLiveHighPassAlpha();
                break;

            case DSP_PARAMETER_EQ_LOW:
                *valueOut = instance->engine.GetLiveEqLow();
                break;

            case DSP_PARAMETER_EQ_MID:
                *valueOut = instance->engine.GetLiveEqMid();
                break;

            case DSP_PARAMETER_EQ_HIGH:
                *valueOut = instance->engine.GetLiveEqHigh();
                break;

            default:
                instance->lastError = "Unknown DSP parameter ID.";
                return DSP_BACKEND_INVALID_ARGUMENT;
            }

            instance->lastError.clear();
            return DSP_BACKEND_OK;
        }
        catch (...)
        {
            instance->lastError = "Unexpected exception while reading a parameter.";
            return DSP_BACKEND_ERROR;
        }
    }

    DSPBackendResult SetGraphLinks(
        DSPBackendInstance* instance,
        const DSPGraphLink* links,
        uint32_t linkCount)
    {
        if (instance == nullptr || (linkCount > 0 && links == nullptr))
        {
            return DSP_BACKEND_INVALID_ARGUMENT;
        }

        if (!instance->initialized)
        {
            instance->lastError = "Initialize the backend before setting graph links.";
            return DSP_BACKEND_NOT_INITIALIZED;
        }

        try
        {
            std::vector<std::pair<int, int>> engineLinks;
            engineLinks.reserve(linkCount);

            for (uint32_t i = 0; i < linkCount; ++i)
            {
                engineLinks.emplace_back(
                    static_cast<int>(links[i].sourceNode),
                    static_cast<int>(links[i].destinationNode));
            }

            instance->engine.SetGraphLinks(engineLinks);
            instance->lastError.clear();
            return DSP_BACKEND_OK;
        }
        catch (...)
        {
            instance->lastError = "Unexpected exception while updating graph links.";
            return DSP_BACKEND_ERROR;
        }
    }

    DSPBackendResult AddDSPModule(
        DSPBackendInstance* instance,
        int32_t nodeId,
        const char* moduleName)
    {
        if (instance == nullptr || moduleName == nullptr || moduleName[0] == '\0')
        {
            return DSP_BACKEND_INVALID_ARGUMENT;
        }

        if (!instance->initialized)
        {
            instance->lastError = "Initialize the backend before adding DSP modules.";
            return DSP_BACKEND_NOT_INITIALIZED;
        }

        try
        {
            if (!instance->engine.AddDSPModule(nodeId, moduleName))
            {
                instance->lastError = "Failed to create or register the DSP module.";
                return DSP_BACKEND_ERROR;
            }

            instance->lastError.clear();
            return DSP_BACKEND_OK;
        }
        catch (...)
        {
            instance->lastError = "Unexpected exception while adding a DSP module.";
            return DSP_BACKEND_ERROR;
        }
    }

    uint32_t CopyVisualizerData(
        DSPBackendInstance* instance,
        float* destination,
        uint32_t capacity)
    {
        if (instance == nullptr || (capacity > 0 && destination == nullptr))
        {
            return 0;
        }

        if (!instance->initialized || capacity == 0)
        {
            return 0;
        }

        try
        {
            const std::vector<float>& data =
                instance->engine.GetVisualizerData();

            const size_t count = (std::min)(
                data.size(),
                static_cast<size_t>(capacity));

            std::copy_n(data.begin(), count, destination);
            return static_cast<uint32_t>(count);
        }
        catch (...)
        {
            instance->lastError = "Unable to copy visualizer data.";
            return 0;
        }
    }

    const char* GetLastError(DSPBackendInstance* instance)
    {
        if (instance == nullptr)
        {
            return "Invalid backend instance.";
        }

        return instance->lastError.c_str();
    }

    const DSPBackendAPI g_backendAPI =
    {
        DSP_BACKEND_API_VERSION,
        sizeof(DSPBackendAPI),
        &CreateInstance,
        &DestroyInstance,
        &InitializeInstance,
        &StartInstance,
        &StopInstance,
        &SetParameter,
        &GetParameter,
        &SetGraphLinks,
        &AddDSPModule,
        &CopyVisualizerData,
        &GetLastError
    };
}

extern "C"
DSP_BACKEND_EXPORT
const DSPBackendAPI* DSPBackend_GetAPI(uint32_t requestedApiVersion)
{
    if (requestedApiVersion != DSP_BACKEND_API_VERSION)
    {
        return nullptr;
    }

    return &g_backendAPI;
}
