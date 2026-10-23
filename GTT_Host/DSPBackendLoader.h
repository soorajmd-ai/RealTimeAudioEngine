
#pragma once

#include "api/DSPBackendAPI.h"

#include <Windows.h>
#include <string>
#include <utility>
#include <vector>

class DSPBackendLoader
{
public:
    DSPBackendLoader() = default;
    ~DSPBackendLoader();

    DSPBackendLoader(const DSPBackendLoader&) = delete;
    DSPBackendLoader& operator=(const DSPBackendLoader&) = delete;

    // DLL loading and unloading
    bool Load(const wchar_t* dllPath);
    bool LoadFromExecutableDirectory(
        const wchar_t* dllFileName = L"CoreDSP.dll");
    void Unload();

    bool IsLoaded() const;
    const std::string& GetError() const;

    const DSPBackendAPI* GetAPI() const;
    DSPBackendInstance* GetInstance() const;

    // Engine lifecycle
    bool Initialize();
    bool Run();
    void Shutdown();

    // Live parameters
    void SetLiveGain(float value);
    float GetLiveGain() const;

    void SetLiveDelayFeedback(float value);
    float GetLiveDelayFeedback() const;

    void SetLiveLowPassAlpha(float value);
    float GetLiveLowPassAlpha() const;

    void SetLiveHighPassAlpha(float value);
    float GetLiveHighPassAlpha() const;

    void SetLiveEQLow(float value);
    float GetLiveEQLow() const;

    void SetLiveEQMid(float value);
    float GetLiveEQMid() const;

    void SetLiveEQHigh(float value);
    float GetLiveEQHigh() const;

    // Graph and visualizer
    void SetGraphLinks(
        const std::vector<std::pair<int, int>>& links);
    bool AddDSPModule(int nodeId, const std::string& moduleName);

    const std::vector<float>& GetVisualizerData();

private:
    bool SetParameter(uint32_t parameterId, float value);
    float GetParameter(uint32_t parameterId) const;
    void CaptureBackendError();

    HMODULE m_module = nullptr;
    const DSPBackendAPI* m_api = nullptr;
    DSPBackendInstance* m_instance = nullptr;

    std::string m_error;
    bool m_initialized = false;
    bool m_running = false;

    std::vector<float> m_visualizerData;
};
