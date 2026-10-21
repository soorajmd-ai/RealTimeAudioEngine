#pragma once
#include "DSPModule.h"
#include <memory>
#include <vector>
#include <map>
#include <mutex>

namespace AudioEngine {
    class DSPChain {
    public:
        DSPChain() = default;
        ~DSPChain() = default;

        // Modules are now added with an explicit Node ID
        void AddModule(int nodeId, std::unique_ptr<DSPModule> module);

        // Updates the wiring dynamically from the UI
        void SetLinks(const std::vector<std::pair<int, int>>& links);

        float Process(float inputSample);

    private:
        std::map<int, std::unique_ptr<DSPModule>> m_modules;
        std::vector<std::pair<int, int>> m_links;
        std::vector<int> m_executionOrder;
        std::mutex m_linkMutex;

        void UpdateExecutionOrder();
    };
}