#include "Engine.h"
#include "GameManager.h"
#include <iostream>

int main() {
    std::cout << "=== GAME BUILD ===" << std::endl;

    Engine::Initialize();
    GameManager::Initialize();

    std::cout << "Game running - ESC to exit" << std::endl;

    while (Engine::IsRunning()) {
        Engine::Update();
        GameManager::Update();

        Engine::StartDraw();
        Engine::Draw();
        Engine::EndDraw();
    }

    GameManager::Shutdown();
    Engine::Shutdown();

    std::cout << "=== Game ended ===" << std::endl;
    return 0;
}