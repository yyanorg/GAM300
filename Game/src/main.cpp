#include "Engine.h"
#include "GameManager.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Starting Game..." << std::endl;

    Engine::Initialize();
    GameManager::Initialize();

    while (Engine::IsRunning()) {
        Engine::Update();
        GameManager::Update();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    GameManager::Shutdown();
    Engine::Shutdown();
    std::cout << "Game ended." << std::endl;
    return 0;
}
