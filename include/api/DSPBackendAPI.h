
#pragma once

#include <stdint.h>

/*
    Stable C-compatible interface between GTT_Host.exe
    and a replaceable DSP backend DLL.

    Do not add C++ classes, STL containers, references,
    exceptions, or C++ standard-library types to this API.
*/

#define DSP_BACKEND_API_VERSION 2u

#if defined(_WIN32) && defined(COREDSP_EXPORTS)
#define DSP_BACKEND_EXPORT __declspec(dllexport)
#else
#define DSP_BACKEND_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct DSPBackendInstance DSPBackendInstance;

    typedef enum DSPBackendResult
    {
        DSP_BACKEND_OK = 0,
        DSP_BACKEND_ERROR = 1,
        DSP_BACKEND_INVALID_ARGUMENT = 2,
        DSP_BACKEND_UNSUPPORTED_VERSION = 3,
        DSP_BACKEND_NOT_INITIALIZED = 4
    } DSPBackendResult;

    typedef enum DSPParameterId
    {
        DSP_PARAMETER_GAIN = 1,
        DSP_PARAMETER_DELAY_FEEDBACK = 2,
        DSP_PARAMETER_LOW_PASS_ALPHA = 3,
        DSP_PARAMETER_HIGH_PASS_ALPHA = 4,
        DSP_PARAMETER_EQ_LOW = 5,
        DSP_PARAMETER_EQ_MID = 6,
        DSP_PARAMETER_EQ_HIGH = 7
    } DSPParameterId;

    typedef struct DSPGraphLink
    {
        int32_t sourceNode;
        int32_t destinationNode;
    } DSPGraphLink;

    typedef struct DSPBackendAPI
    {
        uint32_t apiVersion;
        uint32_t structSize;

        DSPBackendInstance* (*create)(void);

        void (*destroy)(
            DSPBackendInstance* instance);

        DSPBackendResult(*initialize)(
            DSPBackendInstance* instance);

        DSPBackendResult(*start)(
            DSPBackendInstance* instance);

        void (*stop)(
            DSPBackendInstance* instance);

        DSPBackendResult(*setParameter)(
            DSPBackendInstance* instance,
            uint32_t parameterId,
            float value);

        DSPBackendResult(*getParameter)(
            DSPBackendInstance* instance,
            uint32_t parameterId,
            float* valueOut);

        DSPBackendResult(*setGraphLinks)(
            DSPBackendInstance* instance,
            const DSPGraphLink* links,
            uint32_t linkCount);

        DSPBackendResult(*addDSPModule)(
            DSPBackendInstance* instance,
            int32_t nodeId,
            const char* moduleName);

        uint32_t(*copyVisualizerData)(
            DSPBackendInstance* instance,
            float* destination,
            uint32_t capacity);

        const char* (*getLastError)(
            DSPBackendInstance* instance);

    } DSPBackendAPI;

    /*
        The host will find this exported function using
        GetProcAddress after loading the DLL with LoadLibrary.

        The returned API table remains valid while the DLL
        stays loaded.
    */
    DSP_BACKEND_EXPORT
        const DSPBackendAPI* DSPBackend_GetAPI(
            uint32_t requestedApiVersion);

#ifdef __cplusplus
}
#endif

