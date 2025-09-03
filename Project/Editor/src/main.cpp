#include "Engine.h"
#include "GameManager.h"
#include "GUIManager.hpp"
#include <iostream>
#include "imgui.h"


int main() {
    std::cout << "=== EDITOR BUILD ===" << std::endl;

    Engine::Initialize();
    GameManager::Initialize();
	GUIManager::Initialize();
    while (Engine::IsRunning()) {
        Engine::Update();
        GameManager::Update();

        // Render 3D content to FBO
        Engine::StartDraw();
        Engine::Draw();
        GUIManager::Render();
        Engine::EndDraw();
        //ImGui::ShowDemoWindow();
		
        // WindowManager handles buffer swapping for editor
        //WindowManager::SwapBuffers();
    }
	GUIManager::Exit();
    GameManager::Shutdown();
    Engine::Shutdown();

    std::cout << "=== Editor ended ===" << std::endl;
    return 0;
}