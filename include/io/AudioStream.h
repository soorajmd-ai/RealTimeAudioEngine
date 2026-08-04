#pragma once

#include <portaudio.h>

namespace AudioEngine
{
    class AudioStream
    {
    public:

        AudioStream();

        ~AudioStream();

        bool OpenDefaultStream();

        bool Start();

        void Stop();

        void Close();

    private:

        PaStream* m_stream;
    };
}
