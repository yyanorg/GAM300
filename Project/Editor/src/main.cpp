#include "Engine.h"
#include "GameManager.h"
#include <iostream>

int main() {
    std::cout << "=== EDITOR BUILD ===" << std::endl;

    Engine::Initialize();
    GameManager::Initialize();

    while (Engine::IsRunning()) {
        Engine::Update();
        GameManager::Update();

        // Render 3D content to FBO
        Engine::StartDraw();
        Engine::Draw();
        Engine::EndDraw();

        // WindowManager handles buffer swapping for editor
        //WindowManager::SwapBuffers();
    }

    GameManager::Shutdown();
    Engine::Shutdown();

    std::cout << "=== Editor ended ===" << std::endl;
    return 0;
}