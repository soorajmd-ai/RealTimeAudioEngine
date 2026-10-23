
#pragma once

#include "imgui.h"

inline void ApplyAudioStudioTheme()
{
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();

    // Window and control shapes
    style.WindowRounding = 7.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 5.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;

    // Spacing
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.FramePadding = ImVec2(10.0f, 6.0f);
    style.ItemSpacing = ImVec2(9.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(7.0f, 5.0f);
    style.ScrollbarSize = 13.0f;
    style.GrabMinSize = 10.0f;

    ImVec4* c = style.Colors;

    // Main backgrounds
    c[ImGuiCol_WindowBg] = ImVec4(0.055f, 0.070f, 0.095f, 1.00f);
    c[ImGuiCol_ChildBg] = ImVec4(0.065f, 0.082f, 0.110f, 1.00f);
    c[ImGuiCol_PopupBg] = ImVec4(0.075f, 0.090f, 0.120f, 0.98f);

    // Text
    c[ImGuiCol_Text] = ImVec4(0.88f, 0.91f, 0.96f, 1.00f);
    c[ImGuiCol_TextDisabled] = ImVec4(0.43f, 0.49f, 0.58f, 1.00f);

    // Borders
    c[ImGuiCol_Border] = ImVec4(0.16f, 0.20f, 0.26f, 0.85f);
    c[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    c[ImGuiCol_Separator] = ImVec4(0.16f, 0.22f, 0.29f, 1.00f);
    c[ImGuiCol_SeparatorHovered] = ImVec4(0.20f, 0.65f, 0.65f, 0.78f);
    c[ImGuiCol_SeparatorActive] = ImVec4(0.25f, 0.80f, 0.75f, 1.00f);

    // Input fields
    c[ImGuiCol_FrameBg] = ImVec4(0.105f, 0.130f, 0.170f, 1.00f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.21f, 0.27f, 1.00f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.13f, 0.25f, 0.30f, 1.00f);

    // Title bar and menu
    c[ImGuiCol_TitleBg] = ImVec4(0.045f, 0.060f, 0.082f, 1.00f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.075f, 0.105f, 0.140f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.045f, 0.060f, 0.082f, 1.00f);
    c[ImGuiCol_MenuBarBg] = ImVec4(0.075f, 0.095f, 0.125f, 1.00f);

    // Scrollbars
    c[ImGuiCol_ScrollbarBg] = ImVec4(0.045f, 0.060f, 0.080f, 1.00f);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.22f, 0.28f, 0.35f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.29f, 0.38f, 0.46f, 1.00f);
    c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.20f, 0.63f, 0.66f, 1.00f);

    // Teal accent: sliders and checkboxes
    c[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.83f, 0.78f, 1.00f);
    c[ImGuiCol_SliderGrab] = ImVec4(0.20f, 0.70f, 0.69f, 1.00f);
    c[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.91f, 0.82f, 1.00f);

    // Buttons
    c[ImGuiCol_Button] = ImVec4(0.12f, 0.22f, 0.28f, 1.00f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.16f, 0.36f, 0.40f, 1.00f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.12f, 0.48f, 0.49f, 1.00f);

    // Selectable items and headers
    c[ImGuiCol_Header] = ImVec4(0.12f, 0.25f, 0.30f, 1.00f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.15f, 0.36f, 0.40f, 1.00f);
    c[ImGuiCol_HeaderActive] = ImVec4(0.13f, 0.43f, 0.45f, 1.00f);

    // Resize grips
    c[ImGuiCol_ResizeGrip] = ImVec4(0.15f, 0.45f, 0.48f, 0.25f);
    c[ImGuiCol_ResizeGripHovered] = ImVec4(0.20f, 0.70f, 0.69f, 0.70f);
    c[ImGuiCol_ResizeGripActive] = ImVec4(0.28f, 0.86f, 0.79f, 0.95f);

    // Tabs
    c[ImGuiCol_Tab] = ImVec4(0.075f, 0.105f, 0.140f, 1.00f);
    c[ImGuiCol_TabHovered] = ImVec4(0.16f, 0.38f, 0.41f, 1.00f);
    c[ImGuiCol_TabSelected] = ImVec4(0.12f, 0.26f, 0.31f, 1.00f);

    // Docking
    c[ImGuiCol_DockingPreview] = ImVec4(0.20f, 0.78f, 0.72f, 0.45f);
    c[ImGuiCol_DockingEmptyBg] = ImVec4(0.045f, 0.060f, 0.080f, 1.00f);
}

