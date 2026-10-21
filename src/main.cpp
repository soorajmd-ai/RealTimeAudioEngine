#include "core/AudioEngine.h"
#include "Logger.h"

// GUI Libraries
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imnodes.h"

#include <vector>
#include <algorithm>
#include <utility>

struct LinkData
{
    int id;
    int start_attr;
    int end_attr;
};

int main()
{
    AudioEngine::AudioEngine engine;
    if (!engine.Initialize()) return -1;
    engine.Run();

    if (!glfwInit()) return -1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Real-Time Audio Engine - Signal Flow Designer", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImNodes::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImNodes::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    std::vector<LinkData> links;
    int current_link_id = 100;

    // Helper lambda to push the UI graph down into the C++ audio thread
    auto PushGraphToEngine = [&]() {
        std::vector<std::pair<int, int>> active_links;
        for (const auto& l : links) {
            // Elegant conversion: Input/Output pin IDs divided by 10 give the exact Node ID
            active_links.push_back({ l.start_attr / 10, l.end_attr / 10 });
        }
        engine.SetGraphLinks(active_links);
        };

    // Build the default signal chain
    links.push_back({ current_link_id++, 10, 20 }); // Mic -> Gain
    links.push_back({ current_link_id++, 21, 30 }); // Gain -> LowPass
    links.push_back({ current_link_id++, 31, 40 }); // LowPass -> HighPass
    links.push_back({ current_link_id++, 41, 70 }); // HighPass -> Equalizer
    links.push_back({ current_link_id++, 71, 50 }); // Equalizer -> Delay
    links.push_back({ current_link_id++, 51, 60 }); // Delay -> Speaker

    // Send the default graph to the engine immediately
    PushGraphToEngine();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        ImGui::Begin("Signal Flow Designer");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Separator();

        static float currentGain = engine.GetLiveGain();
        static float currentDelay = engine.GetLiveDelayFeedback();
        static float currentLP = engine.GetLiveLowPassAlpha();
        static float currentHP = engine.GetLiveHighPassAlpha();
        static float eqLow = engine.GetLiveEqLow();
        static float eqMid = engine.GetLiveEqMid();
        static float eqHigh = engine.GetLiveEqHigh();

        ImNodes::BeginNodeEditor();

        // Node 1
        ImNodes::BeginNode(1);
        ImNodes::BeginNodeTitleBar(); ImGui::TextUnformatted("Audio In (Mic)"); ImNodes::EndNodeTitleBar();
        ImNodes::BeginOutputAttribute(10); ImGui::Text("Ch 1 Out"); ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        // Node 2
        ImNodes::BeginNode(2);
        ImNodes::BeginNodeTitleBar(); ImGui::TextUnformatted("DSP: Gain"); ImNodes::EndNodeTitleBar();
        ImNodes::BeginInputAttribute(20); ImGui::Text("In"); ImNodes::EndInputAttribute();
        ImGui::PushItemWidth(120.0f);
        if (ImGui::SliderFloat("##gain", &currentGain, 0.0f, 5.0f, "%.2f x")) engine.SetLiveGain(currentGain);
        ImGui::PopItemWidth();
        ImNodes::BeginOutputAttribute(21); ImGui::Text("Out"); ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        // Node 3
        ImNodes::BeginNode(3);
        ImNodes::BeginNodeTitleBar(); ImGui::TextUnformatted("DSP: Low Pass"); ImNodes::EndNodeTitleBar();
        ImNodes::BeginInputAttribute(30); ImGui::Text("In"); ImNodes::EndInputAttribute();
        ImGui::PushItemWidth(120.0f);
        if (ImGui::SliderFloat("##lp", &currentLP, 0.01f, 0.99f, "Alpha %.2f")) engine.SetLiveLowPassAlpha(currentLP);
        ImGui::PopItemWidth();
        ImNodes::BeginOutputAttribute(31); ImGui::Text("Out"); ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        // Node 4
        ImNodes::BeginNode(4);
        ImNodes::BeginNodeTitleBar(); ImGui::TextUnformatted("DSP: High Pass"); ImNodes::EndNodeTitleBar();
        ImNodes::BeginInputAttribute(40); ImGui::Text("In"); ImNodes::EndInputAttribute();
        ImGui::PushItemWidth(120.0f);
        if (ImGui::SliderFloat("##hp", &currentHP, 0.01f, 0.99f, "Alpha %.2f")) engine.SetLiveHighPassAlpha(currentHP);
        ImGui::PopItemWidth();
        ImNodes::BeginOutputAttribute(41); ImGui::Text("Out"); ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        // Node 7 (EQ)
        ImNodes::BeginNode(7);
        ImNodes::BeginNodeTitleBar(); ImGui::TextUnformatted("DSP: 3-Band EQ"); ImNodes::EndNodeTitleBar();
        ImNodes::BeginInputAttribute(70); ImGui::Text("In"); ImNodes::EndInputAttribute();
        ImGui::PushItemWidth(120.0f);
        if (ImGui::SliderFloat("Low (dB)", &eqLow, -15.0f, 15.0f, "%.1f dB")) engine.SetLiveEqLow(eqLow);
        if (ImGui::SliderFloat("Mid (dB)", &eqMid, -15.0f, 15.0f, "%.1f dB")) engine.SetLiveEqMid(eqMid);
        if (ImGui::SliderFloat("High (dB)", &eqHigh, -15.0f, 15.0f, "%.1f dB")) engine.SetLiveEqHigh(eqHigh);
        ImGui::PopItemWidth();
        ImNodes::BeginOutputAttribute(71); ImGui::Text("Out"); ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        // Node 5
        ImNodes::BeginNode(5);
        ImNodes::BeginNodeTitleBar(); ImGui::TextUnformatted("DSP: Delay"); ImNodes::EndNodeTitleBar();
        ImNodes::BeginInputAttribute(50); ImGui::Text("In"); ImNodes::EndInputAttribute();
        ImGui::PushItemWidth(120.0f);
        if (ImGui::SliderFloat("##delay", &currentDelay, 0.0f, 0.99f, "Fb %.2f")) engine.SetLiveDelayFeedback(currentDelay);
        ImGui::PopItemWidth();
        ImNodes::BeginOutputAttribute(51); ImGui::Text("Out"); ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        // Node 6
        ImNodes::BeginNode(6);
        ImNodes::BeginNodeTitleBar(); ImGui::TextUnformatted("Audio Out (Speaker)"); ImNodes::EndNodeTitleBar();
        ImNodes::BeginInputAttribute(60); ImGui::Text("Ch 1 In"); ImNodes::EndInputAttribute();
        const std::vector<float>& audioData = engine.GetVisualizerData();
        ImGui::PushItemWidth(200.0f);
        ImGui::PlotLines("##Scope", audioData.data(), (int)audioData.size(), 0, nullptr, -1.0f, 1.0f, ImVec2(0, 80));
        ImGui::PopItemWidth();
        ImNodes::EndNode();

        for (const auto& link : links)
        {
            ImNodes::Link(link.id, link.start_attr, link.end_attr);
        }

        ImNodes::EndNodeEditor();

        // --- TRACK DYNAMIC CHANGES ---
        bool links_changed = false;
        int start_attr, end_attr;

        // If user draws a new wire
        if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
        {
            links.push_back({ current_link_id++, start_attr, end_attr });
            links_changed = true;
        }

        // If user deletes a wire
        const int num_selected_links = ImNodes::NumSelectedLinks();
        if (num_selected_links > 0 && ImGui::IsKeyReleased(ImGuiKey_Delete))
        {
            std::vector<int> selected_links(num_selected_links);
            ImNodes::GetSelectedLinks(selected_links.data());
            for (int deleted_id : selected_links)
            {
                links.erase(std::remove_if(links.begin(), links.end(),
                    [deleted_id](const LinkData& l) { return l.id == deleted_id; }), links.end());
            }
            links_changed = true;
        }

        // Push the update to the Audio Engine!
        if (links_changed)
        {
            PushGraphToEngine();
        }

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImNodes::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    engine.Shutdown();

    return 0;
}