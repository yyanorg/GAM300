#include "SceneWindow.h"
#include "GraphicsManager.h"
#include "Engine.h"
#include <imgui.h>
#include <iostream>

void SceneWindow::Initialize() {
    std::cout << "[SceneWindow] Initialized" << std::endl;
}

void SceneWindow::RenderSceneWindow(int width, int height) {
    ImGui::Begin("Scene Window");

    // Get available content region
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();

    // Resize engine framebuffer if needed
    //Engine::ResizeFramebuffer((int)contentRegion.x, (int)contentRegion.y);

    // Get the framebuffer texture from engine
    unsigned int texture = Engine::GetFramebufferTexture();

    if (texture != 0) {
        // Display the texture (flipped vertically)
        ImGui::Image((void*)(intptr_t)texture, contentRegion, ImVec2(0, 1), ImVec2(1, 0));
    }
    else {
        ImGui::Text("Engine framebuffer not available");
        std::cout << "no have\n";
    }

    ImGui::End();
}

void SceneWindow::Shutdown() {
    std::cout << "[SceneWindow] Shutdown" << std::endl;
}