#include "Panels/ScenePanel.hpp"
#include "imgui.h"
#include "WindowManager.hpp"

ScenePanel::ScenePanel()
    : EditorPanel("Scene Panel", true) {
}

void ScenePanel::OnImGuiRender() {
    ImGui::Begin("Scene");

    // Get the content region size
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    int sceneViewWidth = (int)viewportPanelSize.x;
    int sceneViewHeight = (int)viewportPanelSize.y;

    // Ensure minimum size
    if (sceneViewWidth < 100) sceneViewWidth = 100;
    if (sceneViewHeight < 100) sceneViewHeight = 100;

    // Render 3D scene using WindowManager
    WindowManager::BeginSceneRender(sceneViewWidth, sceneViewHeight);
    WindowManager::RenderScene();
    WindowManager::EndSceneRender();

    // Get the texture from WindowManager and display it
    unsigned int sceneTexture = WindowManager::GetSceneTexture();
    if (sceneTexture != 0) {
        ImGui::Image(
            (void*)(intptr_t)sceneTexture,
            ImVec2((float)sceneViewWidth, (float)sceneViewHeight),
            ImVec2(0, 1), ImVec2(1, 0)  // Flip Y coordinate for OpenGL
        );
    }
    else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Scene View - Framebuffer not ready");
        ImGui::Text("Size: %dx%d", sceneViewWidth, sceneViewHeight);
    }

    ImGui::End();
}