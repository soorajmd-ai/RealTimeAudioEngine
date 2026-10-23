
#include "dsp/DSPChain.h"

#include <algorithm>
#include <map>
#include <queue>
#include <utility>
#include <vector>

namespace AudioEngine
{
    DSPChain::DSPChain() = default;

    void DSPChain::AddModule(
        int nodeId,
        std::unique_ptr<DSPModule> module)
    {
        // IDs 1 and 6 are reserved for Audio In and Audio Out.
        if (nodeId <= 1 || nodeId == 6 || !module)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(m_moduleRegistrationMutex);

        // Registration is append-only while the chain is alive.
        // Never replace an existing module that the audio thread may use.
        for (const auto& slot : m_modules)
        {
            DSPModule* existing =
                slot.module.load(std::memory_order_acquire);

            if (existing != nullptr &&
                slot.nodeId.load(std::memory_order_relaxed) == nodeId)
            {
                return;
            }
        }

        for (auto& slot : m_modules)
        {
            if (slot.module.load(std::memory_order_acquire) == nullptr)
            {
                // Initialize the owner and ID before publishing the pointer.
                slot.owner = std::move(module);
                slot.nodeId.store(nodeId, std::memory_order_relaxed);

                slot.module.store(
                    slot.owner.get(),
                    std::memory_order_release);

                return;
            }
        }

        // The fixed registry is full. The incoming module is safely
        // destroyed when this function returns.
    }

    void DSPChain::SetLinks(
        const std::vector<std::pair<int, int>>& links)
    {
        std::lock_guard<std::mutex> lock(m_pendingMutex);

        m_pendingLinks.clear();
        m_pendingLinks.reserve(
            std::min(links.size(), MaxLinks));

        for (const auto& link : links)
        {
            const int source = link.first;
            const int destination = link.second;

            if (source < 1 || destination < 2 ||
                source == destination ||
                m_pendingLinks.size() >= MaxLinks)
            {
                continue;
            }

            // Ignore duplicate edges.
            const auto duplicate = std::find(
                m_pendingLinks.begin(),
                m_pendingLinks.end(),
                link);

            if (duplicate != m_pendingLinks.end())
            {
                continue;
            }

            m_pendingLinks.push_back(link);
        }

        UpdateExecutionOrder();
        m_pendingGraphChanged = true;
    }

    void DSPChain::UpdateExecutionOrder()
    {
        // This function runs on the configuration thread while
        // m_pendingMutex is held. It must not run in the audio callback.
        std::map<int, int> inDegree;
        std::map<int, std::vector<int>> adjacency;

        // Register every published module as a graph node.
        for (const auto& slot : m_modules)
        {
            DSPModule* module =
                slot.module.load(std::memory_order_acquire);

            if (module == nullptr)
            {
                continue;
            }

            const int id =
                slot.nodeId.load(std::memory_order_relaxed);

            inDegree.emplace(id, 0);
            adjacency.emplace(id, std::vector<int>{});
        }

        // Include graph edges between registered DSP modules.
        for (const auto& link : m_pendingLinks)
        {
            const int source = link.first;
            const int destination = link.second;

            // Audio In (1) is a source, not a DSP module.
            // Audio Out (6) is a destination, not a DSP module.
            if (source == 6 || destination == 1)
            {
                continue;
            }

            if (source != 1 && inDegree.find(source) == inDegree.end())
            {
                continue;
            }

            if (destination != 6 &&
                inDegree.find(destination) == inDegree.end())
            {
                continue;
            }

            if (source != 1 && destination != 6)
            {
                adjacency[source].push_back(destination);
                ++inDegree[destination];
            }
        }

        std::queue<int> ready;

        for (const auto& entry : inDegree)
        {
            if (entry.second == 0)
            {
                ready.push(entry.first);
            }
        }

        std::vector<int> executionOrder;
        executionOrder.reserve(MaxNodes);

        while (!ready.empty())
        {
            const int current = ready.front();
            ready.pop();

            executionOrder.push_back(current);

            for (const int next : adjacency[current])
            {
                auto degree = inDegree.find(next);

                if (degree != inDegree.end())
                {
                    --degree->second;

                    if (degree->second == 0)
                    {
                        ready.push(next);
                    }
                }
            }
        }

        // A cycle prevents a complete topological ordering.
        // Keep the pending order empty rather than executing a
        // partially ordered graph.
        if (executionOrder.size() != inDegree.size())
        {
            m_pendingExecutionOrder.clear();
            return;
        }

        m_pendingExecutionOrder = std::move(executionOrder);
    }

    void DSPChain::BeginAudioBlock()
    {
        // Never wait for the UI/configuration thread from the callback.
        std::unique_lock<std::mutex> lock(
            m_pendingMutex,
            std::try_to_lock);

        if (!lock.owns_lock() || !m_pendingGraphChanged)
        {
            return;
        }

        m_activeLinkCount = std::min(
            m_pendingLinks.size(),
            MaxLinks);

        for (std::size_t i = 0; i < m_activeLinkCount; ++i)
        {
            m_activeLinks[i] = m_pendingLinks[i];
        }

        m_activeExecutionOrderCount = std::min(
            m_pendingExecutionOrder.size(),
            MaxNodes);

        for (std::size_t i = 0;
            i < m_activeExecutionOrderCount;
            ++i)
        {
            m_activeExecutionOrder[i] =
                m_pendingExecutionOrder[i];
        }

        m_pendingGraphChanged = false;
    }

    DSPModule* DSPChain::FindModule(int nodeId) const
    {
        for (const auto& slot : m_modules)
        {
            DSPModule* module =
                slot.module.load(std::memory_order_acquire);

            if (module != nullptr &&
                slot.nodeId.load(std::memory_order_relaxed) == nodeId)
            {
                return module;
            }
        }

        return nullptr;
    }

    float DSPChain::Process(float inputSample)
    {
        std::array<NodeOutput, MaxNodes> outputs{};
        std::size_t outputCount = 0;

        // Process each DSP node in topological order.
        for (std::size_t i = 0;
            i < m_activeExecutionOrderCount;
            ++i)
        {
            const int nodeId = m_activeExecutionOrder[i];
            float nodeInput = 0.0f;

            // Sum the signals connected to this node's input.
            for (std::size_t linkIndex = 0;
                linkIndex < m_activeLinkCount;
                ++linkIndex)
            {
                const auto& link = m_activeLinks[linkIndex];

                if (link.second != nodeId)
                {
                    continue;
                }

                if (link.first == 1)
                {
                    nodeInput += inputSample;
                    continue;
                }

                for (std::size_t outputIndex = 0;
                    outputIndex < outputCount;
                    ++outputIndex)
                {
                    if (outputs[outputIndex].nodeId == link.first)
                    {
                        nodeInput += outputs[outputIndex].sample;
                        break;
                    }
                }
            }

            DSPModule* module = FindModule(nodeId);

            if (module == nullptr)
            {
                continue;
            }

            const float nodeOutput = module->Process(nodeInput);

            if (outputCount < outputs.size())
            {
                outputs[outputCount].nodeId = nodeId;
                outputs[outputCount].sample = nodeOutput;
                ++outputCount;
            }
        }

        // Sum signals connected to Audio Out (node 6).
        float outputSample = 0.0f;

        for (std::size_t linkIndex = 0;
            linkIndex < m_activeLinkCount;
            ++linkIndex)
        {
            const auto& link = m_activeLinks[linkIndex];

            if (link.second != 6)
            {
                continue;
            }

            if (link.first == 1)
            {
                outputSample += inputSample;
                continue;
            }

            for (std::size_t outputIndex = 0;
                outputIndex < outputCount;
                ++outputIndex)
            {
                if (outputs[outputIndex].nodeId == link.first)
                {
                    outputSample += outputs[outputIndex].sample;
                    break;
                }
            }
        }

        return outputSample;
    }
}
