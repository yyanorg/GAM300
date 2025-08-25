#include "Engine.h"
#include "GameManager.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Starting Editor..." << std::endl;

    Engine::Initialize();
    GameManager::Initialize();

    while (Engine::IsRunning()) {
        Engine::Update();
        GameManager::Update();

        // Editor-specific logic would go here
        // (ImGui rendering, scene editing, etc.)

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    GameManager::Shutdown();
    Engine::Shutdown();
    std::cout << "Editor ended." << std::endl;
    return 0;
}