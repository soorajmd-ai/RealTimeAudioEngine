#include "io/PortAudioWrapper.h"
#include "Logger.h"

#include <string>

namespace AudioEngine
{
    PortAudioWrapper::PortAudioWrapper()
    {}

    PortAudioWrapper::~PortAudioWrapper()
    {
        Terminate();
    }

    bool PortAudioWrapper::Initialize()
    {
        if (m_initialized)
        {
            return true;
        }

        const PaError error = Pa_Initialize();

        if (error != paNoError)
        {
            Logger::Error(Pa_GetErrorText(error));
            return false;
        }

        m_initialized = true;

        Logger::Info("PortAudio initialized successfully.");
        Logger::Info(Pa_GetVersionText());

        PrintAudioDevices();

        return true;
    }

    void PortAudioWrapper::Terminate()
    {
        if (!m_initialized)
        {
            return;
        }

        const PaError error = Pa_Terminate();

        // Clear the state to prevent repeated termination attempts.
        m_initialized = false;

        if (error != paNoError)
        {
            Logger::Error(Pa_GetErrorText(error));
            return;
        }

        Logger::Info("PortAudio terminated.");
    }

    void PortAudioWrapper::PrintAudioDevices() const
    {
        if (!m_initialized)
        {
            return;
        }

        const int deviceCount = Pa_GetDeviceCount();

        if (deviceCount < 0)
        {
            Logger::Error("Failed to retrieve audio devices.");
            return;
        }

        Logger::Info("========== Available Audio Devices ==========");

        for (int i = 0; i < deviceCount; ++i)
        {
            const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);

            if (deviceInfo != nullptr)
            {
                const std::string info =
                    std::to_string(i) + " : " + deviceInfo->name;

                Logger::Info(info);
            }
        }
    }
}
