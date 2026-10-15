#include "core/AudioEngine.h"
#include "Logger.h"

// GUI Libraries
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imnodes.h"

int main()
{
    // 1. Initialize and Start the Audio Engine
    AudioEngine::AudioEngine engine;
    if (!engine.Initialize())
    {
        AudioEngine::Logger::Error("Failed to initialize Audio Engine.");
        return -1;
    }
    engine.Run();

    // 2. Initialize GLFW
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

    // 3. Initialize Dear ImGui & ImNodes
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

    // 4. Main Application Loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        ImGui::Begin("Signal Flow Designer");
        ImGui::Text("Drag nodes to organize your DSP Pipeline.");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Separator();

        // Fetch live values
        static float currentGain = engine.GetLiveGain();
        static float currentDelay = engine.GetLiveDelayFeedback();
        static float currentLP = engine.GetLiveLowPassAlpha();
        static float currentHP = engine.GetLiveHighPassAlpha();

        static float eqLow = engine.GetLiveEqLow();
        static float eqMid = engine.GetLiveEqMid();
        static float eqHigh = engine.GetLiveEqHigh();

        // --- START NODE GRAPH ---
        ImNodes::BeginNodeEditor();

        // NODE 1: Audio Input
        ImNodes::BeginNode(1);
        ImNodes::BeginNodeTitleBar(); ImGui::TextUnformatted("Audio In (Mic)"); ImNodes::EndNodeTitleBar();
        ImNodes::BeginOutputAttribute(10); ImGui::Text("Ch 1 Out"); ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        // NODE 2: DSP Gain
        ImNodes::BeginNode(2);
        ImNodes::BeginNodeTitleBar(); ImGui::TextUnformatted("DSP: Gain"); ImNodes::EndNodeTitleBar();
        ImNodes::BeginInputAttribute(20); ImGui::Text("In"); ImNodes::EndInputAttribute();
        ImGui::PushItemWidth(120.0f);
        if (ImGui::SliderFloat("##gain", &currentGain, 0.0f, 5.0f, "%.2f x")) engine.SetLiveGain(currentGain);
        ImGui::PopItemWidth();
        ImNodes::BeginOutputAttribute(21); ImGui::Text("Out"); ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        // NODE 3: Low Pass Filter
        ImNodes::BeginNode(3);
        ImNodes::BeginNodeTitleBar(); ImGui::TextUnformatted("DSP: Low Pass"); ImNodes::EndNodeTitleBar();
        ImNodes::BeginInputAttribute(30); ImGui::Text("In"); ImNodes::EndInputAttribute();
        ImGui::PushItemWidth(120.0f);
        if (ImGui::SliderFloat("##lp", &currentLP, 0.01f, 0.99f, "Alpha %.2f")) engine.SetLiveLowPassAlpha(currentLP);
        ImGui::PopItemWidth();
        ImNodes::BeginOutputAttribute(31); ImGui::Text("Out"); ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        // NODE 4: High Pass Filter
        ImNodes::BeginNode(4);
        ImNodes::BeginNodeTitleBar(); ImGui::TextUnformatted("DSP: High Pass"); ImNodes::EndNodeTitleBar();
        ImNodes::BeginInputAttribute(40); ImGui::Text("In"); ImNodes::EndInputAttribute();
        ImGui::PushItemWidth(120.0f);
        if (ImGui::SliderFloat("##hp", &currentHP, 0.01f, 0.99f, "Alpha %.2f")) engine.SetLiveHighPassAlpha(currentHP);
        ImGui::PopItemWidth();
        ImNodes::BeginOutputAttribute(41); ImGui::Text("Out"); ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        // NODE 5: 3-Band Equalizer
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

        // NODE 6: Delay
        ImNodes::BeginNode(5);
        ImNodes::BeginNodeTitleBar(); ImGui::TextUnformatted("DSP: Delay"); ImNodes::EndNodeTitleBar();
        ImNodes::BeginInputAttribute(50); ImGui::Text("In"); ImNodes::EndInputAttribute();
        ImGui::PushItemWidth(120.0f);
        if (ImGui::SliderFloat("##delay", &currentDelay, 0.0f, 0.99f, "Fb %.2f")) engine.SetLiveDelayFeedback(currentDelay);
        ImGui::PopItemWidth();
        ImNodes::BeginOutputAttribute(51); ImGui::Text("Out"); ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        // NODE 7: Audio Output
        ImNodes::BeginNode(6);
        ImNodes::BeginNodeTitleBar(); ImGui::TextUnformatted("Audio Out (Speaker)"); ImNodes::EndNodeTitleBar();
        ImNodes::BeginInputAttribute(60); ImGui::Text("Ch 1 In"); ImNodes::EndInputAttribute();
        ImNodes::EndNode();

        // --- DRAW WIRES ---
        ImNodes::Link(100, 10, 20); // Mic -> Gain
        ImNodes::Link(101, 21, 30); // Gain -> LowPass
        ImNodes::Link(102, 31, 40); // LowPass -> HighPass
        ImNodes::Link(103, 41, 70); // HighPass -> Equalizer
        ImNodes::Link(105, 71, 50); // Equalizer -> Delay
        ImNodes::Link(104, 51, 60); // Delay -> Speaker

        ImNodes::EndNodeEditor();
        ImGui::End();

        // --- RENDER ---
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