#include "Engine.h"
#include "GameManager.h"
#include "EditorManager.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Starting Editor..." << std::endl;

    Engine::Initialize();
    GameManager::Initialize();

    // Small delay to ensure window is fully ready
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EditorManager::Initialize();

    while (Engine::IsRunning()) {
        Engine::Update();
        GameManager::Update();

        // Editor rendering
        EditorManager::Update();
        EditorManager::Render();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    EditorManager::Shutdown();
    GameManager::Shutdown();
    Engine::Shutdown();

    std::cout << "Editor ended." << std::endl;
    return 0;
}