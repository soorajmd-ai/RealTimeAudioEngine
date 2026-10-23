#include "DSPBackendLoader.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <cstring>
#include <cctype>

// GUI Libraries
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imnodes.h"
#include "ImGuiTheme.h"

struct LinkData
{
    int id;
    int start_attr;
    int end_attr;
};

struct DSPNodeData
{
    int id;
    std::string name;
    std::string moduleName;
};

int main()
{
    // ---------------------------------------------------------
    // 1. Load and start the DSP backend dynamically
    // ---------------------------------------------------------
    DSPBackendLoader backend;

    if (!backend.LoadFromExecutableDirectory())
    {
        std::cerr << "Failed to load DSP backend: "
            << backend.GetError() << '\n';
        return -1;
    }

    if (!backend.Initialize())
    {
        std::cerr << "Failed to initialize DSP backend: "
            << backend.GetError() << '\n';
        backend.Unload();
        return -1;
    }

    if (!backend.Run())
    {
        std::cerr << "Failed to start DSP backend: "
            << backend.GetError() << '\n';
        backend.Shutdown();
        backend.Unload();
        return -1;
    }

    // ---------------------------------------------------------
    // 2. Initialize GLFW and create the application window
    // ---------------------------------------------------------
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW.\n";
        backend.Shutdown();
        backend.Unload();
        return -1;
    }

    const char* glsl_version = "#version 130";

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(
        1280,
        720,
        "Real-Time Audio Engine - Signal Flow Designer",
        nullptr,
        nullptr
    );

    if (!window)
    {
        std::cerr << "Failed to create the application window.\n";
        glfwTerminate();
        backend.Shutdown();
        backend.Unload();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // ---------------------------------------------------------
    // 3. Initialize ImGui and ImNodes
    // ---------------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ApplyAudioStudioTheme();
    ImNodes::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImNodes::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // ---------------------------------------------------------
    // 4. Signal-flow graph
    // ---------------------------------------------------------
    std::vector<LinkData> links;
    int current_link_id = 100;
    std::string graphStatus = "Ready. Signal graph initialized.";
    std::vector<DSPNodeData> dynamicNodes;
    int nextDynamicNodeId = 8;

    // Push the UI graph into the dynamically loaded DSP backend.
    auto PushGraphToEngine = [&]()
        {
            std::vector<std::pair<int, int>> active_links;

            for (const auto& link : links)
            {
                // Pin IDs / 10 map to the corresponding node IDs.
                active_links.push_back({
                    link.start_attr / 10,
                    link.end_attr / 10
                    });
            }

            backend.SetGraphLinks(active_links);
        };


    // Validate graph pins and connections before updating the backend.
    auto GetPinDirection = [&](int pin, int& nodeId, bool& isOutput)
        {
            if (pin <= 0)
                return false;

            nodeId = pin / 10;

            const bool isDynamic = std::any_of(
                dynamicNodes.begin(), dynamicNodes.end(),
                [&](const DSPNodeData& node) { return node.id == nodeId; });

            if (nodeId < 1 || (nodeId > 7 && !isDynamic))
                return false;

            if (pin == 10)
            {
                nodeId = 1;
                isOutput = true;
                return true;
            }

            if (pin % 10 == 0)
            {
                isOutput = false;
                return pin == nodeId * 10 &&
                    (nodeId == 2 || nodeId == 3 || nodeId == 4 ||
                     nodeId == 5 || nodeId == 6 || nodeId == 7 ||
                     isDynamic);
            }

            if (pin % 10 == 1)
            {
                isOutput = true;
                return pin == nodeId * 10 + 1 &&
                    (nodeId == 2 || nodeId == 3 || nodeId == 4 ||
                     nodeId == 5 || nodeId == 7 || isDynamic);
            }

            return false;
        };

    auto NormalizeLink = [&](const LinkData& link, int& sourceNode,
                             int& destinationNode)
        {
            int firstNode = 0;
            int secondNode = 0;
            bool firstIsOutput = false;
            bool secondIsOutput = false;

            if (!GetPinDirection(link.start_attr, firstNode, firstIsOutput) ||
                !GetPinDirection(link.end_attr, secondNode, secondIsOutput))
                return false;

            if (firstIsOutput == secondIsOutput)
                return false;

            sourceNode = firstIsOutput ? firstNode : secondNode;
            destinationNode = firstIsOutput ? secondNode : firstNode;
            return sourceNode != destinationNode;
        };

    auto WouldCreateCycle = [&](int sourceNode, int destinationNode)
        {
            // A cycle exists if the destination can already reach the source.
            std::vector<int> pending{ destinationNode };
            std::vector<int> visited;

            while (!pending.empty())
            {
                const int current = pending.back();
                pending.pop_back();

                if (current == sourceNode)
                    return true;

                if (std::find(visited.begin(), visited.end(), current)
                    != visited.end())
                    continue;

                visited.push_back(current);

                for (const auto& existing : links)
                {
                    int existingSource = 0;
                    int existingDestination = 0;

                    if (NormalizeLink(existing, existingSource,
                                      existingDestination) &&
                        existingSource == current)
                    {
                        pending.push_back(existingDestination);
                    }
                }
            }

            return false;
        };

    // Build the default signal chain.
    links.push_back({ current_link_id++, 10, 20 }); // Mic -> Gain
    links.push_back({ current_link_id++, 21, 30 }); // Gain -> LowPass
    links.push_back({ current_link_id++, 31, 40 }); // LowPass -> HighPass
    links.push_back({ current_link_id++, 41, 70 }); // HighPass -> Equalizer
    links.push_back({ current_link_id++, 71, 50 }); // Equalizer -> Delay
    links.push_back({ current_link_id++, 51, 60 }); // Delay -> Speaker

    // Send the initial graph to the backend.
    PushGraphToEngine();

    // ---------------------------------------------------------
    // 5. Initialize live DSP parameter controls
    // ---------------------------------------------------------
    float currentGain = backend.GetLiveGain();
    float currentDelay = backend.GetLiveDelayFeedback();
    float currentLP = backend.GetLiveLowPassAlpha();
    float currentHP = backend.GetLiveHighPassAlpha();

    float eqLow = backend.GetLiveEQLow();
    float eqMid = backend.GetLiveEQMid();
    float eqHigh = backend.GetLiveEQHigh();

    // Default inspector selection: Gain.
    int selectedNodeId = 2;


    // Status displayed in the GUI.
    std::string backendStatus = "DSP backend running.";

    // Reload the backend without closing the GUI.

    // Reload the backend without closing the GUI.
    auto ReloadDSPBackend = [&]()
        {
            // Find the directory containing GTT_Host.exe.
            wchar_t executablePath[MAX_PATH] = {};

            const DWORD pathLength = ::GetModuleFileNameW(
                nullptr, executablePath, MAX_PATH);

            if (pathLength == 0 || pathLength >= MAX_PATH)
            {
                backendStatus =
                    "Reload failed: could not determine the executable directory.";
                return;
            }

            std::wstring directory(executablePath);

            const std::size_t separator =
                directory.find_last_of(L"\\/");

            if (separator == std::wstring::npos)
            {
                backendStatus =
                    "Reload failed: executable directory is invalid.";
                return;
            }

            directory.resize(separator + 1);

            const std::wstring sourcePath =
                directory + L"CoreDSP.dll";

            const std::wstring candidatePath =
                directory + L"CoreDSP_candidate.dll";

            const std::wstring runtimePath =
                directory + L"CoreDSP_runtime.dll";

            const std::wstring backupPath =
                directory + L"CoreDSP_previous.dll";

            // Step 1: Copy the latest build to a separate candidate file.
            if (!::CopyFileW(
                sourcePath.c_str(),
                candidatePath.c_str(),
                FALSE))
            {
                backendStatus =
                    "Reload failed: could not prepare candidate DLL. "
                    "Windows error: " +
                    std::to_string(::GetLastError());
                return;
            }

            // Step 2: Validate the candidate API without opening audio.
            {
                DSPBackendLoader validator;

                if (!validator.Load(candidatePath.c_str()))
                {
                    backendStatus =
                        "Reload rejected: candidate DLL validation failed. " +
                        validator.GetError();
                    return;
                }

                // Destroy the temporary instance and release the DLL.
                validator.Unload();
            }

            // Step 3: Back up the currently deployed runtime DLL.
            // The current backend is still running at this point.
            if (!::CopyFileW(
                runtimePath.c_str(),
                backupPath.c_str(),
                FALSE))
            {
                backendStatus =
                    "Reload failed: could not back up the current backend. "
                    "Windows error: " +
                    std::to_string(::GetLastError());
                return;
            }

            // Restore the previous DLL and restart it if switching fails.
            auto RestorePreviousBackend =
                [&](const std::string& reason)
                {
                    // Ensure no module is holding the runtime DLL open.
                    backend.Shutdown();
                    backend.Unload();

                    // Restore the last working DLL on disk.
                    if (!::CopyFileW(
                        backupPath.c_str(),
                        runtimePath.c_str(),
                        FALSE))
                    {
                        backendStatus =
                            "Reload failed; restoring the previous DLL also failed. "
                            "Windows error: " +
                            std::to_string(::GetLastError());
                        return;
                    }

                    if (!backend.Load(runtimePath.c_str()))
                    {
                        backendStatus =
                            "Reload failed; previous DLL could not be loaded: " +
                            backend.GetError();
                        return;
                    }

                    if (!backend.Initialize())
                    {
                        const std::string recoveryError =
                            backend.GetError();

                        backend.Unload();

                        backendStatus =
                            "Reload failed; previous backend initialization failed: " +
                            recoveryError;
                        return;
                    }

                    // Restore the GUI's current parameter values.
                    backend.SetLiveGain(currentGain);
                    backend.SetLiveDelayFeedback(currentDelay);
                    backend.SetLiveLowPassAlpha(currentLP);
                    backend.SetLiveHighPassAlpha(currentHP);

                    backend.SetLiveEQLow(eqLow);
                    backend.SetLiveEQMid(eqMid);
                    backend.SetLiveEQHigh(eqHigh);

                    // Restore the current GUI graph.
                    PushGraphToEngine();

                    if (!backend.Run())
                    {
                        const std::string recoveryError =
                            backend.GetError();

                        backend.Shutdown();
                        backend.Unload();

                        backendStatus =
                            "Reload failed; previous backend could not restart: " +
                            recoveryError;
                        return;
                    }

                    backendStatus =
                        "Reload failed; previous backend restored. Reason: " +
                        reason;
                };

            // Step 4: Stop the old engine before opening the new audio stream.
            backend.Shutdown();
            backend.Unload();

            // Step 5: Deploy the validated candidate as the runtime DLL.
            if (!::CopyFileW(
                candidatePath.c_str(),
                runtimePath.c_str(),
                FALSE))
            {
                const std::string reason =
                    "Could not deploy candidate DLL. Windows error: " +
                    std::to_string(::GetLastError());

                RestorePreviousBackend(reason);
                return;
            }

            // Step 6: Load the replacement.
            if (!backend.Load(runtimePath.c_str()))
            {
                const std::string reason = backend.GetError();

                RestorePreviousBackend(reason);
                return;
            }

            // Step 7: Initialize the replacement audio engine.
            if (!backend.Initialize())
            {
                const std::string reason = backend.GetError();

                RestorePreviousBackend(reason);
                return;
            }

            // Step 8: Restore live parameters and graph connections.
            backend.SetLiveGain(currentGain);
            backend.SetLiveDelayFeedback(currentDelay);
            backend.SetLiveLowPassAlpha(currentLP);
            backend.SetLiveHighPassAlpha(currentHP);

            backend.SetLiveEQLow(eqLow);
            backend.SetLiveEQMid(eqMid);
            backend.SetLiveEQHigh(eqHigh);

            PushGraphToEngine();

            // Step 9: Start the replacement engine.
            if (!backend.Run())
            {
                const std::string reason = backend.GetError();

                RestorePreviousBackend(reason);
                return;
            }

            backendStatus = "DSP backend reloaded successfully.";
        };



    // ---------------------------------------------------------
    // 6. Main application loop
    // ---------------------------------------------------------
    bool showSignalFlowGraph = true;
    bool showNodeInspector = true;
    bool showAudioMonitor = true;
    bool resetWorkspaceRequested = false;
    bool workspaceInitializationChecked = false;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit"))
                {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Signal Flow Graph", nullptr, &showSignalFlowGraph);
                ImGui::MenuItem("Node Inspector", nullptr, &showNodeInspector);
                ImGui::MenuItem("Audio Monitor", nullptr, &showAudioMonitor);
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Workspace"))
                {
                    resetWorkspaceRequested = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Plugins"))
            {
                ImGui::MenuItem("DSP Backend");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help"))
            {
                ImGui::MenuItem("Real-Time Audio Engine");
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        ImGuiViewport* mainViewport = ImGui::GetMainViewport();
        const ImGuiID dockspaceId = mainViewport->ID;

        if (!workspaceInitializationChecked || resetWorkspaceRequested)
        {
            ImGuiDockNode* existingDockspace =
                ImGui::DockBuilderGetNode(dockspaceId);

            const bool needsDefaultLayout =
                resetWorkspaceRequested ||
                existingDockspace == nullptr ||
                existingDockspace->Size.x < 500.0f ||
                existingDockspace->Size.y < 300.0f;

            if (needsDefaultLayout)
            {
                ImGui::DockBuilderRemoveNode(dockspaceId);
                ImGui::DockBuilderAddNode(
                    dockspaceId, ImGuiDockNodeFlags_DockSpace
                );
                ImGui::DockBuilderSetNodeSize(
                    dockspaceId, mainViewport->WorkSize
                );

                ImGuiID toolbarNode = 0;
                ImGuiID mainAreaNode = 0;
                ImGuiID inspectorNode = 0;
                ImGuiID contentNode = 0;
                ImGuiID monitorNode = 0;
                ImGuiID graphNode = 0;
                ImGuiID toolboxNode = 0;

                // Reserve the top strip for the application toolbar.
                ImGui::DockBuilderSplitNode(
                    dockspaceId,
                    ImGuiDir_Up,
                    0.075f,
                    &toolbarNode,
                    &mainAreaNode
                );

                // Right-side node inspector.
                ImGui::DockBuilderSplitNode(
                    mainAreaNode,
                    ImGuiDir_Right,
                    0.26f,
                    &inspectorNode,
                    &contentNode
                );

                // Bottom monitoring panel.
                ImGui::DockBuilderSplitNode(
                    contentNode,
                    ImGuiDir_Down,
                    0.27f,
                    &monitorNode,
                    &graphNode
                );

                // Left-side DSP module library.
                ImGui::DockBuilderSplitNode(
                    graphNode,
                    ImGuiDir_Left,
                    0.20f,
                    &toolboxNode,
                    &graphNode
                );

                // Dock existing application panels.
                ImGui::DockBuilderDockWindow(
                    "Audio Engine Toolbar",
                    toolbarNode
                );

                ImGui::DockBuilderDockWindow(
                    "DSP Toolbox",
                    toolboxNode
                );

                ImGui::DockBuilderDockWindow(
                    "Signal Flow Graph",
                    graphNode
                );

                ImGui::DockBuilderDockWindow(
                    "Node Inspector",
                    inspectorNode
                );

                ImGui::DockBuilderDockWindow(
                    "Audio Monitor",
                    monitorNode
                );


                ImGui::DockBuilderFinish(dockspaceId);

                showSignalFlowGraph = true;
                showNodeInspector = true;
                showAudioMonitor = true;
            }

            workspaceInitializationChecked = true;
            resetWorkspaceRequested = false;
        }

        ImGui::DockSpaceOverViewport(dockspaceId, mainViewport);

        ImGui::Begin("Audio Engine Toolbar");

        ImGui::TextUnformatted("REAL-TIME AUDIO ENGINE");
        ImGui::SameLine();

        if (ImGui::Button("Reload DSP"))
        {
            ReloadDSPBackend();
        }

        ImGui::SameLine();
        ImGui::TextColored(
            backendStatus.find("failed") != std::string::npos ||
            backendStatus.find("Failed") != std::string::npos
            ? ImVec4(1.0f, 0.35f, 0.30f, 1.0f)
            : ImVec4(0.35f, 0.85f, 0.55f, 1.0f),
            "%s",
            backendStatus.c_str()
        );

        ImGui::End();

        // ---------------------------------------------------------
        // DSP TOOLBOX - professional module browser
        // ---------------------------------------------------------
        ImGui::Begin("DSP Toolbox");

        ImGui::TextColored(
            ImVec4(0.30f, 0.86f, 0.80f, 1.0f),
            "DSP MODULE LIBRARY"
        );

        ImGui::TextDisabled("Build your signal-processing chain");
        ImGui::Separator();
        ImGui::Spacing();

        // Search field for the module library.
        static char dspSearch[64] = "";
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint(
            "##DSPSearch",
            "Search DSP modules...",
            dspSearch,
            sizeof(dspSearch)
        );

        ImGui::Spacing();

        auto AddDSPNode = [&](const char* displayName,
            const char* moduleName)
            {
                const int nodeId = nextDynamicNodeId++;

                // Preserve existing backend registration.
                backend.AddDSPModule(nodeId, moduleName);

                // Preserve existing graph-node registration.
                dynamicNodes.push_back({
                    nodeId,
                    displayName,
                    moduleName
                    });

                // Place the new node in the graph.
                ImNodes::SetNodeGridSpacePos(
                    nodeId,
                    ImVec2(
                        100.0f + 40.0f * nodeId,
                        100.0f + 30.0f * nodeId
                    )
                );
            };

        auto DrawDSPButton = [&](const char* label,
            const char* moduleName,
            const char* category,
            ImVec4 accent)
            {
                // Filter module entries using the search field.
                if (dspSearch[0] != '\0' &&
                    strstr(label, dspSearch) == nullptr &&
                    strstr(category, dspSearch) == nullptr)
                {
                    // Retry case-insensitively below.
                    std::string lowerLabel = label;
                    std::string lowerCategory = category;
                    std::string lowerSearch = dspSearch;

                    auto ToLower = [](std::string& value)
                        {
                            for (char& ch : value)
                            {
                                ch = static_cast<char>(
                                    std::tolower(
                                        static_cast<unsigned char>(ch)
                                    )
                                    );
                            }
                        };

                    ToLower(lowerLabel);
                    ToLower(lowerCategory);
                    ToLower(lowerSearch);

                    if (lowerLabel.find(lowerSearch) == std::string::npos &&
                        lowerCategory.find(lowerSearch) == std::string::npos)
                    {
                        return;
                    }
                }

                ImGui::PushID(moduleName);

                ImGui::PushStyleColor(
                    ImGuiCol_Button,
                    ImVec4(0.09f, 0.13f, 0.19f, 1.0f)
                );

                ImGui::PushStyleColor(
                    ImGuiCol_ButtonHovered,
                    ImVec4(
                        accent.x * 0.40f,
                        accent.y * 0.40f,
                        accent.z * 0.40f,
                        1.0f
                    )
                );

                ImGui::PushStyleColor(
                    ImGuiCol_ButtonActive,
                    accent
                );

                if (ImGui::Button(label, ImVec2(-1.0f, 39.0f)))
                {
                    AddDSPNode(label, moduleName);
                }

                ImGui::PopStyleColor(3);
                ImGui::PopID();
            };

        // Audio processing modules.
        if (ImGui::CollapsingHeader(
            "  FILTERS",
            ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Spacing();

            DrawDSPButton(
                "+  Low Pass Filter",
                "LowPass",
                "Filters",
                ImVec4(0.20f, 0.75f, 0.80f, 1.0f)
            );

            DrawDSPButton(
                "+  High Pass Filter",
                "HighPass",
                "Filters",
                ImVec4(0.20f, 0.75f, 0.80f, 1.0f)
            );

            ImGui::Spacing();
        }

        if (ImGui::CollapsingHeader(
            "  DYNAMICS & LEVEL",
            ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Spacing();

            DrawDSPButton(
                "+  Gain",
                "Gain",
                "Dynamics",
                ImVec4(0.25f, 0.72f, 0.62f, 1.0f)
            );

            ImGui::Spacing();
        }

        if (ImGui::CollapsingHeader(
            "  EQUALIZATION",
            ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Spacing();

            DrawDSPButton(
                "+  3-Band Equalizer",
                "Equalizer",
                "Equalization",
                ImVec4(0.65f, 0.40f, 0.90f, 1.0f)
            );

            ImGui::Spacing();
        }

        if (ImGui::CollapsingHeader(
            "  TIME-BASED EFFECTS",
            ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Spacing();

            DrawDSPButton(
                "+  Delay",
                "Delay",
                "Time-Based Effects",
                ImVec4(0.90f, 0.62f, 0.25f, 1.0f)
            );

            ImGui::Spacing();
        }

        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextDisabled("MODULES AVAILABLE");
        ImGui::Text(
            "%d DSP modules",
            5
        );

        ImGui::TextDisabled(
            "Select a module to add it to the graph."
        );

        ImGui::End();


        ImGui::Begin("Signal Flow Graph", &showSignalFlowGraph);


        // Premium signal-flow graph styling
        ImNodes::PushColorStyle(
            ImNodesCol_NodeBackground,
            IM_COL32(25, 34, 49, 255));

        ImNodes::PushColorStyle(
            ImNodesCol_NodeBackgroundHovered,
            IM_COL32(39, 57, 77, 255));

        ImNodes::PushColorStyle(
            ImNodesCol_NodeBackgroundSelected,
            IM_COL32(32, 67, 77, 255));

        ImNodes::PushColorStyle(
            ImNodesCol_NodeOutline,
            IM_COL32(65, 103, 126, 255));

        ImNodes::PushColorStyle(
            ImNodesCol_TitleBar,
            IM_COL32(25, 65, 80, 255));

        ImNodes::PushColorStyle(
            ImNodesCol_TitleBarHovered,
            IM_COL32(32, 100, 112, 255));

        ImNodes::PushColorStyle(
            ImNodesCol_TitleBarSelected,
            IM_COL32(34, 125, 132, 255));

        ImNodes::PushColorStyle(
            ImNodesCol_Link,
            IM_COL32(65, 190, 205, 255));

        ImNodes::PushColorStyle(
            ImNodesCol_LinkHovered,
            IM_COL32(120, 235, 225, 255));

        ImNodes::PushColorStyle(
            ImNodesCol_LinkSelected,
            IM_COL32(255, 190, 90, 255));


        // Persistent graph feedback, kept above the node canvas.
        ImGui::TextUnformatted("GRAPH STATUS");
        ImGui::SameLine();
        ImGui::TextWrapped("%s", graphStatus.c_str());
        ImGui::Separator();

        ImNodes::BeginNodeEditor();


        // -----------------------------------------------------
        // Node 1: Audio Input
        // -----------------------------------------------------
        ImNodes::BeginNode(1);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("Audio In (Mic)");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginOutputAttribute(10);
        ImGui::Text("Ch 1 Out");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();

        // -----------------------------------------------------
        // Node 2: Gain
        // -----------------------------------------------------
        ImNodes::BeginNode(2);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("DSP: Gain");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(20);
        ImGui::Text("In");
        ImNodes::EndInputAttribute();



        ImNodes::BeginOutputAttribute(21);
        ImGui::Text("Out");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();

        // -----------------------------------------------------
        // Node 3: Low-Pass Filter
        // -----------------------------------------------------
        ImNodes::BeginNode(3);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("DSP: Low Pass");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(30);
        ImGui::Text("In");
        ImNodes::EndInputAttribute();



        ImNodes::BeginOutputAttribute(31);
        ImGui::Text("Out");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();

        // -----------------------------------------------------
        // Node 4: High-Pass Filter
        // -----------------------------------------------------
        ImNodes::BeginNode(4);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("DSP: High Pass");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(40);
        ImGui::Text("In");
        ImNodes::EndInputAttribute();



        ImNodes::BeginOutputAttribute(41);
        ImGui::Text("Out");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();

        // -----------------------------------------------------
        // Node 7: Three-Band Equalizer
        // -----------------------------------------------------
        ImNodes::BeginNode(7);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("DSP: 3-Band EQ");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(70);
        ImGui::Text("In");
        ImNodes::EndInputAttribute();



        ImNodes::BeginOutputAttribute(71);
        ImGui::Text("Out");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();

        // -----------------------------------------------------
        // Node 5: Delay
        // -----------------------------------------------------
        ImNodes::BeginNode(5);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("DSP: Delay");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(50);
        ImGui::Text("In");
        ImNodes::EndInputAttribute();



        ImNodes::BeginOutputAttribute(51);
        ImGui::Text("Out");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();

        // -----------------------------------------------------
        // Node 6: Audio Output and Visualizer
        // -----------------------------------------------------
        ImNodes::BeginNode(6);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("Audio Out (Speaker)");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(60);
        ImGui::Text("Ch 1 In");
        ImNodes::EndInputAttribute();

        ImGui::TextUnformatted("Output channel 1");

        ImNodes::EndNode();

        // ---------------------------------------------------------
// Dynamically created DSP nodes
// ---------------------------------------------------------
        for (const auto& node : dynamicNodes)
        {
            const int inputPin = node.id * 10;
            const int outputPin = node.id * 10 + 1;

            ImNodes::BeginNode(node.id);

            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted(node.name.c_str());
            ImNodes::EndNodeTitleBar();

            ImNodes::BeginInputAttribute(inputPin);
            ImGui::TextUnformatted("In");
            ImNodes::EndInputAttribute();

            ImNodes::BeginOutputAttribute(outputPin);
            ImGui::TextUnformatted("Out");
            ImNodes::EndOutputAttribute();

            ImNodes::EndNode();
        }

        // Draw the existing graph connections.
        for (const auto& link : links)
        {
            ImNodes::Link(
                link.id,
                link.start_attr,
                link.end_attr
            );
        }

        ImNodes::EndNodeEditor();

        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();


        const int selectedNodeCount = ImNodes::NumSelectedNodes();
        if (selectedNodeCount > 0)
        {
            std::vector<int> selectedNodes(selectedNodeCount);
            ImNodes::GetSelectedNodes(selectedNodes.data());
            selectedNodeId = selectedNodes[0];
        }


        // -----------------------------------------------------
        // 7. Handle graph-link creation and deletion
        // -----------------------------------------------------
        bool links_changed = false;
        int start_attr = 0;
        int end_attr = 0;

        // Validate a new connection before accepting it.
        if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
        {
            LinkData candidate{ current_link_id, start_attr, end_attr };
            int sourceNode = 0;
            int destinationNode = 0;

            if (!NormalizeLink(candidate, sourceNode, destinationNode))
            {
                graphStatus =
                    "Connection rejected: invalid pins or incompatible "
                    "input/output directions.";
                std::cerr << "[Graph] " << graphStatus << '\n';
            }
            else
            {
                const bool duplicate = std::any_of(
                    links.begin(), links.end(),
                    [&](const LinkData& existing)
                    {
                        int existingSource = 0;
                        int existingDestination = 0;

                        return NormalizeLink(existing, existingSource,
                                             existingDestination) &&
                               existingSource == sourceNode &&
                               existingDestination == destinationNode;
                    });

                if (duplicate)
                {
                    graphStatus =
                        "Connection rejected: this connection already exists.";
                    std::cerr << "[Graph] " << graphStatus << '\n';
                }
                else if (WouldCreateCycle(sourceNode, destinationNode))
                {
                    graphStatus =
                        "Connection rejected: it would create a signal-flow cycle.";
                    std::cerr << "[Graph] " << graphStatus << '\n';
                }
                else
                {
                    // Store links consistently as output pin -> input pin.
                    if (start_attr % 10 == 0 && start_attr != 10)
                        std::swap(candidate.start_attr, candidate.end_attr);

                    links.push_back(candidate);
                    ++current_link_id;
                    graphStatus = "Connection added successfully.";
                    links_changed = true;
                }
            }
        }

        // If the user selects and deletes connections.
        const int num_selected_links = ImNodes::NumSelectedLinks();

        if (num_selected_links > 0 &&
            ImGui::IsKeyReleased(ImGuiKey_Delete))
        {
            std::vector<int> selected_links(num_selected_links);
            ImNodes::GetSelectedLinks(selected_links.data());

            for (int deleted_id : selected_links)
            {
                links.erase(
                    std::remove_if(
                        links.begin(),
                        links.end(),
                        [deleted_id](const LinkData& link)
                        {
                            return link.id == deleted_id;
                        }
                    ),
                    links.end()
                );
            }

            links_changed = true;
        }

        // Send graph changes to the DSP backend.
        if (links_changed)
        {
            PushGraphToEngine();
        }

        ImGui::End();


        // -----------------------------------------------------
        // Node Inspector: controls for the selected DSP node
        // -----------------------------------------------------
        ImGui::Begin("Node Inspector", &showNodeInspector);

        ImGui::TextUnformatted("SELECTED NODE");
        ImGui::Separator();

        switch (selectedNodeId)
        {
        case 1:
            ImGui::TextUnformatted("Audio In (Mic)");
            ImGui::TextWrapped("Audio input node. No parameters are exposed yet.");
            break;

        case 2:
            ImGui::TextUnformatted("Gain");
            ImGui::Text("Linear gain: %.2f x", currentGain);
            if (ImGui::SliderFloat(
                "Gain", &currentGain, 0.0f, 5.0f, "%.2f x"))
            {
                backend.SetLiveGain(currentGain);
            }
            break;

        case 3:
            ImGui::TextUnformatted("Low-Pass Filter");
            ImGui::Text("Alpha: %.2f", currentLP);
            if (ImGui::SliderFloat(
                "LP Alpha", &currentLP, 0.01f, 0.99f, "%.2f"))
            {
                backend.SetLiveLowPassAlpha(currentLP);
            }
            break;

        case 4:
            ImGui::TextUnformatted("High-Pass Filter");
            ImGui::Text("Alpha: %.2f", currentHP);
            if (ImGui::SliderFloat(
                "HP Alpha", &currentHP, 0.01f, 0.99f, "%.2f"))
            {
                backend.SetLiveHighPassAlpha(currentHP);
            }
            break;

        case 7:
            ImGui::TextUnformatted("Three-Band Equalizer");
            if (ImGui::SliderFloat(
                "Low (dB)", &eqLow, -15.0f, 15.0f, "%.1f dB"))
            {
                backend.SetLiveEQLow(eqLow);
            }
            if (ImGui::SliderFloat(
                "Mid (dB)", &eqMid, -15.0f, 15.0f, "%.1f dB"))
            {
                backend.SetLiveEQMid(eqMid);
            }
            if (ImGui::SliderFloat(
                "High (dB)", &eqHigh, -15.0f, 15.0f, "%.1f dB"))
            {
                backend.SetLiveEQHigh(eqHigh);
            }
            break;

        case 5:
            ImGui::TextUnformatted("Delay");
            ImGui::Text("Feedback: %.2f", currentDelay);
            if (ImGui::SliderFloat(
                "Feedback", &currentDelay, 0.0f, 0.99f, "%.2f"))
            {
                backend.SetLiveDelayFeedback(currentDelay);
            }
            break;

        case 6:
            ImGui::TextUnformatted("Audio Out (Speaker)");
            ImGui::TextWrapped(
                "Output node. The waveform scope remains visible in the graph.");
            break;

            // Dynamically created DSP nodes
        default:
        {
            auto nodeIt = std::find_if(
                dynamicNodes.begin(),
                dynamicNodes.end(),
                [&](const DSPNodeData& node)
                {
                    return node.id == selectedNodeId;
                }
            );

            if (nodeIt != dynamicNodes.end())
            {
                ImGui::Text("%s", nodeIt->name.c_str());

                ImGui::TextWrapped(
                    "Runtime DSP module: %s",
                    nodeIt->moduleName.c_str()
                );

                ImGui::TextWrapped(
                    "Dedicated per-node parameter controls "
                    "are not implemented yet."
                );
            }
            else
            {
                ImGui::TextUnformatted(
                    "Select a node in the graph."
                );
            }

            break;
        }
        } // End switch (selectedNodeId)

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("DSP BACKEND");
        ImGui::TextWrapped("%s", backendStatus.c_str());

        ImGui::End();

        // -----------------------------------------------------
        // Audio Monitor: waveform from the active DSP backend
        // -----------------------------------------------------
        ImGui::Begin("Audio Monitor", &showAudioMonitor);

        const std::vector<float>& monitorData = backend.GetVisualizerData();

        ImGui::TextUnformatted("OUTPUT WAVEFORM");
        ImGui::SameLine();
        ImGui::TextDisabled("Live backend samples");

        if (!monitorData.empty())
        {
            ImGui::PlotLines(
                "##OutputWaveform",
                monitorData.data(),
                static_cast<int>(monitorData.size()),
                0,
                nullptr,
                -1.0f,
                1.0f,
                ImVec2(-1.0f, 150.0f)
            );
        }
        else
        {
            ImGui::TextDisabled("Waiting for audio samples...");
        }

        ImGui::Separator();
        ImGui::TextDisabled(
            "Spectrum analysis will be added in a later monitoring milestone."
        );

        ImGui::End();

        // -----------------------------------------------------
        // 8. Render the application
        // -----------------------------------------------------
        ImGui::Render();

        int display_w = 0;
        int display_h = 0;

        glfwGetFramebufferSize(window, &display_w, &display_h);

        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // ---------------------------------------------------------
    // 9. Clean shutdown
    // ---------------------------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImNodes::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    backend.Shutdown();
    backend.Unload();

    return 0;
}
