#include "io/PortAudioWrapper.h"
#include "Logger.h"

namespace AudioEngine
{

    PortAudioWrapper::PortAudioWrapper()
    {}

    PortAudioWrapper::~PortAudioWrapper()
    {}

    bool PortAudioWrapper::Initialize()
    {
        PaError error = Pa_Initialize();

        if (error != paNoError)
        {
            Logger::Error(Pa_GetErrorText(error));
            return false;
        }

        Logger::Info("PortAudio initialized successfully.");

        Logger::Info(Pa_GetVersionText());

        PrintAudioDevices();

        return true;
    }

    void PortAudioWrapper::Terminate()
    {
        Pa_Terminate();

        Logger::Info("PortAudio terminated.");
    }

    void PortAudioWrapper::PrintAudioDevices() const
    {
       
        int deviceCount = Pa_GetDeviceCount();

        if (deviceCount < 0)
        {
            Logger::Error("Failed to retrieve audio devices.");
            return;
        }

        Logger::Info("========== Available Audio Devices ==========");

        for (int i = 0; i < deviceCount; ++i)
        {
            const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);

            if (deviceInfo)
            {
                std::string info = std::to_string(i) +
                    " : " +
                    deviceInfo->name;

                Logger::Info(info);
            }
        }
    }

}