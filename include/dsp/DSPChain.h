
#pragma once

#include "DSPModule.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

namespace AudioEngine
{
    class DSPChain
    {
    public:
        static constexpr std::size_t MaxNodes = 64;
        static constexpr std::size_t MaxLinks = 128;

        DSPChain();

        ~DSPChain() = default;

        // Registers a DSP module under a node ID.
        // Modules must not be replaced or removed while audio processing
        // may still be using them.
        void AddModule(
            int nodeId,
            std::unique_ptr<DSPModule> module);

        // Updates the pending graph configuration.
        void SetLinks(
            const std::vector<std::pair<int, int>>& links);

        // Called once at the start of each audio callback.
        // Does not wait for the UI thread.
        void BeginAudioBlock();

        // Called for each sample; does not acquire a mutex.
        float Process(float inputSample);

    private:
        struct NodeOutput
        {
            int nodeId = 0;
            float sample = 0.0f;
        };

        struct ModuleSlot
        {
            // The module owner is written during registration and remains
            // unchanged while the audio thread can access the module.
            std::unique_ptr<DSPModule> owner;

            // Publishing this pointer makes the initialized slot visible
            // to the audio thread.
            std::atomic<DSPModule*> module{ nullptr };

            // Read only after module has been acquired as non-null.
            std::atomic<int> nodeId{ -1 };
        };

        // Fixed-capacity registry: no mutable map lookup in the audio path.
        std::array<ModuleSlot, MaxNodes> m_modules{};

        // Protects module registration on non-audio threads.
        std::mutex m_moduleRegistrationMutex;

        // UI-thread graph configuration.
        std::vector<std::pair<int, int>> m_pendingLinks;
        std::vector<int> m_pendingExecutionOrder;
        std::mutex m_pendingMutex;
        bool m_pendingGraphChanged = false;

        // Audio-thread-owned graph snapshot.
        std::array<std::pair<int, int>, MaxLinks> m_activeLinks{};
        std::size_t m_activeLinkCount = 0;

        std::array<int, MaxNodes> m_activeExecutionOrder{};
        std::size_t m_activeExecutionOrderCount = 0;

        // Finds a published module without acquiring a mutex.
        DSPModule* FindModule(int nodeId) const;

        // Rebuilds the pending execution order.
        // Called while m_pendingMutex is held.
        void UpdateExecutionOrder();
    };
}
