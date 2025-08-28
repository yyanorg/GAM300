#include "Engine.h"
#include "GameManager.h"
#include "UIManager.h"
#include "SceneWindow.h"
#include "WindowManager.h"
#include <iostream>

int main() {
    std::cout << "=== EDITOR BUILD ===" << std::endl;

    Engine::Initialize();
    GameManager::Initialize();
    UIManager::Initialize();
    SceneWindow::Initialize();

    std::cout << "Editor running with proper architecture!" << std::endl;

    while (Engine::IsRunning()) {
        Engine::Update();
        GameManager::Update();

        // Render 3D content to FBO
        Engine::StartDraw();
        Engine::Draw();

        // Render UI
        UIManager::StartRender();
        UIManager::Render();
        SceneWindow::RenderSceneWindow(WindowManager::GetWindowWidth(), WindowManager::GetWindowHeight());
        UIManager::EndRender();

        Engine::EndDraw();

        // WindowManager handles buffer swapping for editor
        //WindowManager::SwapBuffers();
    }

    SceneWindow::Shutdown();
    UIManager::Shutdown();
    GameManager::Shutdown();
    Engine::Shutdown();

    std::cout << "=== Editor ended ===" << std::endl;
    return 0;
}