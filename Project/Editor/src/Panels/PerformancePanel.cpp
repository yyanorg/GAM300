#include "Panels/PerformancePanel.hpp"
#include "imgui.h"
#include "WindowManager.hpp"

PerformancePanel::PerformancePanel()
    : EditorPanel("Performance", true) {
}

void PerformancePanel::OnImGuiRender() {
    if (ImGui::Begin(m_Name.c_str(), &m_IsOpen)) {
        ImGui::Text("FPS: %.1f", WindowManager::getFps());
        ImGui::Text("Delta Time: %.3f ms", WindowManager::getDeltaTime() * 1000.0);
    }
    ImGui::End();
}