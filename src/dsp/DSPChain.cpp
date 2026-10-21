#include "dsp/DSPChain.h"
#include <queue>

namespace AudioEngine {

    void DSPChain::AddModule(int nodeId, std::unique_ptr<DSPModule> module) {
        std::lock_guard<std::mutex> lock(m_linkMutex);
        m_modules[nodeId] = std::move(module);
    }

    void DSPChain::SetLinks(const std::vector<std::pair<int, int>>& links) {
        std::lock_guard<std::mutex> lock(m_linkMutex);
        m_links = links;
        UpdateExecutionOrder();
    }

    void DSPChain::UpdateExecutionOrder() {
        m_executionOrder.clear();
        std::map<int, int> inDegree;
        std::map<int, std::vector<int>> adjList;

        // Node 1 is Mic, Node 6 is Speaker. Initialize in-degrees.
        inDegree[1] = 0;
        inDegree[6] = 0;
        for (const auto& pair : m_modules) {
            inDegree[pair.first] = 0;
        }

        // Build adjacency list and calculate in-degrees
        for (const auto& link : m_links) {
            adjList[link.first].push_back(link.second);
            inDegree[link.second]++;
        }

        // Kahn's Algorithm for Topological Sorting
        std::queue<int> zeroInDegree;
        for (const auto& pair : inDegree) {
            if (pair.second == 0) zeroInDegree.push(pair.first);
        }

        while (!zeroInDegree.empty()) {
            int node = zeroInDegree.front();
            zeroInDegree.pop();
            m_executionOrder.push_back(node);

            for (int neighbor : adjList[node]) {
                inDegree[neighbor]--;
                if (inDegree[neighbor] == 0) {
                    zeroInDegree.push(neighbor);
                }
            }
        }
    }

    float DSPChain::Process(float inputSample) {
        // Try to lock. If the UI is currently wiring the graph, we skip one sample 
        // to strictly avoid blocking the real-time audio thread (Lock-Free design).
        std::unique_lock<std::mutex> lock(m_linkMutex, std::try_to_lock);
        if (!lock.owns_lock()) return 0.0f;

        // Fast fixed-size array to hold the audio sample at each node (Supports up to Node ID 15)
        float nodeOutputs[15] = { 0.0f };
        nodeOutputs[1] = inputSample; // Node 1 is Mic

        // Execute nodes in topological order
        for (int node : m_executionOrder) {
            if (node == 1 || node == 6) continue; // Skip raw I/O nodes

            // 1. Sum incoming audio from all wires connected to this node's input
            float inSample = 0.0f;
            for (const auto& link : m_links) {
                if (link.second == node) {
                    inSample += nodeOutputs[link.first];
                }
            }

            // 2. Process the audio through the DSP module
            if (m_modules.find(node) != m_modules.end()) {
                nodeOutputs[node] = m_modules[node]->Process(inSample);
            }
        }

        // Final Output: Sum all wires connected to Node 6 (Speaker)
        float outSample = 0.0f;
        for (const auto& link : m_links) {
            if (link.second == 6) {
                outSample += nodeOutputs[link.first];
            }
        }

        return outSample;
    }

} // namespace AudioEngine