#pragma once

#include <array>
#include <atomic>
#include <cstdint>

namespace AudioEngine
{
    // Single-producer/single-consumer queue for visualizer samples.
    //
    // Producer: PortAudio callback.
    // Consumer: UI thread.
    //
    // Push never waits and never allocates. If the queue is full,
    // the sample is dropped.
    class VisualizerSPSCQueue
    {
    public:
        static constexpr uint32_t Capacity = 8192;

        VisualizerSPSCQueue()
            : m_writeIndex(0),
              m_readIndex(0)
        {
        }

        bool Push(float sample)
        {
            const uint32_t writeIndex =
                m_writeIndex.load(std::memory_order_relaxed);

            const uint32_t readIndex =
                m_readIndex.load(std::memory_order_acquire);

            if (writeIndex - readIndex >= Capacity)
            {
                return false;
            }

            m_samples[writeIndex % Capacity] = sample;

            m_writeIndex.store(
                writeIndex + 1,
                std::memory_order_release);

            return true;
        }

        bool Pop(float& sample)
        {
            const uint32_t readIndex =
                m_readIndex.load(std::memory_order_relaxed);

            const uint32_t writeIndex =
                m_writeIndex.load(std::memory_order_acquire);

            if (readIndex == writeIndex)
            {
                return false;
            }

            sample = m_samples[readIndex % Capacity];

            m_readIndex.store(
                readIndex + 1,
                std::memory_order_release);

            return true;
        }

    private:
        std::array<float, Capacity> m_samples;

        alignas(64) std::atomic<uint32_t> m_writeIndex;
        alignas(64) std::atomic<uint32_t> m_readIndex;
    };
}
