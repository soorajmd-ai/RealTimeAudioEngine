#pragma once

#include "buffer/RingBuffer.h"
#include "dsp/DSPChain.h"
#include "dsp/Gain.h"
#include "dsp/LowPassFilter.h"
#include "dsp/Delay.h"
#include "dsp/HighPassFilter.h"
#include "dsp/Equalizer.h"

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

        void SetHighPassAlpha(float alpha);

        void SetDelaySamples(int samples);

        void SetDelayFeedback(float feedback);

        void EnableGain(bool enable);

        void EnableLowPass(bool enable);

        void EnableHighPass(bool enable);

        void EnableDelay(bool enable);

        void EnableEqualizer(bool enable);

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

        HighPassFilter m_highPass;

        Delay m_delay;

        Equalizer m_equalizer;

        PaStream* m_stream;

        bool m_gainEnabled;

        bool m_lowPassEnabled;

        bool m_highPassEnabled;

        bool m_delayEnabled;

        bool m_equalizerEnabled;
    };
}
