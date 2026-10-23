
#include "DSPBackendLoader.h"

#include <algorithm>
#include <sstream>

namespace
{
    using GetBackendAPIFunction =
        const DSPBackendAPI* (*)(uint32_t);
}

DSPBackendLoader::~DSPBackendLoader()
{
    Unload();
}

bool DSPBackendLoader::Load(const wchar_t* dllPath)
{
    Unload();
    m_error.clear();

    if (dllPath == nullptr || dllPath[0] == L'\0')
    {
        m_error = "The backend DLL path is empty.";
        return false;
    }

    m_module = ::LoadLibraryW(dllPath);

    if (m_module == nullptr)
    {
        const DWORD errorCode = ::GetLastError();

        std::ostringstream message;
        message << "LoadLibraryW failed. Windows error: "
            << errorCode;

        m_error = message.str();
        return false;
    }

    auto getAPI = reinterpret_cast<GetBackendAPIFunction>(
        ::GetProcAddress(m_module, "DSPBackend_GetAPI"));

    if (getAPI == nullptr)
    {
        m_error = "The DLL does not export DSPBackend_GetAPI.";
        Unload();
        return false;
    }

    m_api = getAPI(DSP_BACKEND_API_VERSION);

    if (m_api == nullptr)
    {
        m_error =
            "The backend does not support the requested API version.";
        Unload();
        return false;
    }

    if (m_api->apiVersion != DSP_BACKEND_API_VERSION ||
        m_api->structSize < sizeof(DSPBackendAPI) ||
        m_api->create == nullptr ||
        m_api->destroy == nullptr ||
        m_api->initialize == nullptr ||
        m_api->start == nullptr ||
        m_api->stop == nullptr ||
        m_api->setParameter == nullptr ||
        m_api->getParameter == nullptr ||
        m_api->setGraphLinks == nullptr ||
        m_api->copyVisualizerData == nullptr ||
        m_api->getLastError == nullptr)
    {
        m_error =
            "The backend API table is incomplete or incompatible.";
        Unload();
        return false;
    }

    m_instance = m_api->create();

    if (m_instance == nullptr)
    {
        m_error = "The backend could not create an instance.";
        Unload();
        return false;
    }

    return true;
}


bool DSPBackendLoader::LoadFromExecutableDirectory(
    const wchar_t* dllFileName)
{
    // Release the previously loaded runtime copy before replacing it.
    Unload();
    m_error.clear();

    if (dllFileName == nullptr || dllFileName[0] == L'\0')
    {
        m_error = "The backend DLL filename is empty.";
        return false;
    }

    wchar_t executablePath[MAX_PATH] = {};

    const DWORD length = ::GetModuleFileNameW(
        nullptr, executablePath, MAX_PATH);

    if (length == 0 || length >= MAX_PATH)
    {
        m_error =
            "Could not determine the executable directory.";
        return false;
    }

    // Find the directory containing GTT_Host.exe.
    std::wstring directory(executablePath);

    const std::size_t separator =
        directory.find_last_of(L"\\/");

    if (separator == std::wstring::npos)
    {
        m_error =
            "Could not determine the executable directory.";
        return false;
    }

    directory.resize(separator + 1);

    // The original build output remains available to Visual Studio.
    const std::wstring sourcePath =
        directory + dllFileName;

    // The application loads this separate runtime copy.
    const std::wstring runtimePath =
        directory + L"CoreDSP_runtime.dll";

    // Replace the old runtime copy with the latest build.
    if (!::CopyFileW(
        sourcePath.c_str(),
        runtimePath.c_str(),
        FALSE))
    {
        const DWORD errorCode = ::GetLastError();

        std::ostringstream message;
        message << "Could not copy the DSP DLL to its runtime "
            << "file. Windows error: " << errorCode;

        m_error = message.str();
        return false;
    }

    // Load the copy, not the original build output.
    return Load(runtimePath.c_str());
}


void DSPBackendLoader::Unload()
{
    // Never unload a DLL while one of its instances still exists.
    if (m_instance != nullptr && m_api != nullptr)
    {
        if (m_initialized)
        {
            m_api->stop(m_instance);
        }

        m_api->destroy(m_instance);
    }

    m_instance = nullptr;
    m_api = nullptr;
    m_initialized = false;
    m_running = false;

    if (m_module != nullptr)
    {
        ::FreeLibrary(m_module);
        m_module = nullptr;
    }

    m_visualizerData.clear();
}

bool DSPBackendLoader::IsLoaded() const
{
    return m_module != nullptr &&
        m_api != nullptr &&
        m_instance != nullptr;
}

const std::string& DSPBackendLoader::GetError() const
{
    return m_error;
}

const DSPBackendAPI* DSPBackendLoader::GetAPI() const
{
    return m_api;
}

DSPBackendInstance* DSPBackendLoader::GetInstance() const
{
    return m_instance;
}

void DSPBackendLoader::CaptureBackendError()
{
    if (m_api != nullptr && m_instance != nullptr)
    {
        const char* message =
            m_api->getLastError(m_instance);

        if (message != nullptr && message[0] != '\0')
        {
            m_error = message;
            return;
        }
    }

    if (m_error.empty())
    {
        m_error = "The DSP backend operation failed.";
    }
}

bool DSPBackendLoader::Initialize()
{
    if (!IsLoaded())
    {
        m_error = "Cannot initialize: no backend is loaded.";
        return false;
    }

    if (m_initialized)
    {
        return true;
    }

    const DSPBackendResult result =
        m_api->initialize(m_instance);

    if (result != DSP_BACKEND_OK)
    {
        CaptureBackendError();
        return false;
    }

    m_initialized = true;
    m_error.clear();
    return true;
}

bool DSPBackendLoader::Run()
{
    if (!IsLoaded() || !m_initialized)
    {
        m_error =
            "Cannot start: the backend is not initialized.";
        return false;
    }

    if (m_running)
    {
        return true;
    }

    const DSPBackendResult result =
        m_api->start(m_instance);

    if (result != DSP_BACKEND_OK)
    {
        CaptureBackendError();
        return false;
    }

    m_running = true;
    m_error.clear();
    return true;
}

void DSPBackendLoader::Shutdown()
{
    if (m_instance != nullptr &&
        m_api != nullptr &&
        m_initialized)
    {
        m_api->stop(m_instance);
    }

    m_running = false;
    m_initialized = false;
}

bool DSPBackendLoader::SetParameter(
    uint32_t parameterId, float value)
{
    if (!IsLoaded() || !m_initialized)
    {
        m_error =
            "Cannot set a parameter: the backend is not initialized.";
        return false;
    }

    const DSPBackendResult result =
        m_api->setParameter(m_instance, parameterId, value);

    if (result != DSP_BACKEND_OK)
    {
        CaptureBackendError();
        return false;
    }

    return true;
}

float DSPBackendLoader::GetParameter(
    uint32_t parameterId) const
{
    if (!IsLoaded() || !m_initialized)
    {
        return 0.0f;
    }

    float value = 0.0f;

    const DSPBackendResult result =
        m_api->getParameter(m_instance, parameterId, &value);

    return result == DSP_BACKEND_OK ? value : 0.0f;
}

void DSPBackendLoader::SetLiveGain(float value)
{
    SetParameter(DSP_PARAMETER_GAIN, value);
}

float DSPBackendLoader::GetLiveGain() const
{
    return GetParameter(DSP_PARAMETER_GAIN);
}

void DSPBackendLoader::SetLiveDelayFeedback(float value)
{
    SetParameter(DSP_PARAMETER_DELAY_FEEDBACK, value);
}

float DSPBackendLoader::GetLiveDelayFeedback() const
{
    return GetParameter(DSP_PARAMETER_DELAY_FEEDBACK);
}

void DSPBackendLoader::SetLiveLowPassAlpha(float value)
{
    SetParameter(DSP_PARAMETER_LOW_PASS_ALPHA, value);
}

float DSPBackendLoader::GetLiveLowPassAlpha() const
{
    return GetParameter(DSP_PARAMETER_LOW_PASS_ALPHA);
}

void DSPBackendLoader::SetLiveHighPassAlpha(float value)
{
    SetParameter(DSP_PARAMETER_HIGH_PASS_ALPHA, value);
}

float DSPBackendLoader::GetLiveHighPassAlpha() const
{
    return GetParameter(DSP_PARAMETER_HIGH_PASS_ALPHA);
}

void DSPBackendLoader::SetLiveEQLow(float value)
{
    SetParameter(DSP_PARAMETER_EQ_LOW, value);
}

float DSPBackendLoader::GetLiveEQLow() const
{
    return GetParameter(DSP_PARAMETER_EQ_LOW);
}

void DSPBackendLoader::SetLiveEQMid(float value)
{
    SetParameter(DSP_PARAMETER_EQ_MID, value);
}

float DSPBackendLoader::GetLiveEQMid() const
{
    return GetParameter(DSP_PARAMETER_EQ_MID);
}

void DSPBackendLoader::SetLiveEQHigh(float value)
{
    SetParameter(DSP_PARAMETER_EQ_HIGH, value);
}

float DSPBackendLoader::GetLiveEQHigh() const
{
    return GetParameter(DSP_PARAMETER_EQ_HIGH);
}

void DSPBackendLoader::SetGraphLinks(
    const std::vector<std::pair<int, int>>& links)
{
    if (!IsLoaded() || !m_initialized)
    {
        m_error =
            "Cannot update graph: the backend is not initialized.";
        return;
    }

    std::vector<DSPGraphLink> apiLinks;
    apiLinks.reserve(links.size());

    for (const auto& link : links)
    {
        apiLinks.push_back({
            static_cast<int32_t>(link.first),
            static_cast<int32_t>(link.second)
            });
    }

    const DSPBackendResult result = m_api->setGraphLinks(
        m_instance,
        apiLinks.empty() ? nullptr : apiLinks.data(),
        static_cast<uint32_t>(apiLinks.size()));

    if (result != DSP_BACKEND_OK)
    {
        CaptureBackendError();
    }
}

bool DSPBackendLoader::AddDSPModule(
    int nodeId,
    const std::string& moduleName)
{
    if (!m_api || !m_instance || !m_initialized)
    {
        m_error = "Backend is not initialized.";
        return false;
    }

    if (!m_api->addDSPModule)
    {
        m_error = "Backend does not support adding DSP modules.";
        return false;
    }

    const DSPBackendResult result =
        m_api->addDSPModule(m_instance, nodeId, moduleName.c_str());

    if (result != DSP_BACKEND_OK)
    {
        CaptureBackendError();
        return false;
    }

    m_error.clear();
    return true;
}

const std::vector<float>& DSPBackendLoader::GetVisualizerData()
{
    if (!IsLoaded() || !m_initialized)
    {
        m_visualizerData.clear();
        return m_visualizerData;
    }

    // Match the 512-sample visualizer buffer in AudioStream.
    constexpr uint32_t capacity = 512;

    m_visualizerData.resize(capacity);

    const uint32_t count = m_api->copyVisualizerData(
        m_instance,
        m_visualizerData.data(),
        capacity);

    m_visualizerData.resize(
        (std::min)(count, capacity));

    return m_visualizerData;
}
