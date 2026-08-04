#pragma once

#include <portaudio.h>

namespace AudioEngine
{
    class PortAudioWrapper
    {
    public:

        PortAudioWrapper();

        ~PortAudioWrapper();

        bool Initialize();

        void Terminate();
    };
}
