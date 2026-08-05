#pragma once

#include "buffer/RingBuffer.h"
#include "dsp/DSPChain.h"
#include "dsp/Gain.h"
#include "dsp/LowPassFilter.h"

#include <portaudio.h>

namespace AudioEngine
{
    class AudioStream
    {
    public:

        AudioStream();

        ~AudioStream();

        bool OpenDefaultStream();

        void SetGain(float gain);

        void SetLowPassAlpha(float alpha);

        bool Start();

        void Stop();

        void Close();

        int ProcessAudio(
            const void* input,
            void* output,
            unsigned long frameCount);

    private:

        RingBuffer<float> m_ringBuffer;

        DSPChain m_dspChain;

        Gain m_gain;

        LowPassFilter m_lowPass;

        PaStream* m_stream;
    };
}
