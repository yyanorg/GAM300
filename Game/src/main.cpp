//#include "Engine.h"
//#include <iostream>
//#include <thread>
//#include <chrono>
//
//int main() {
//    std::cout << "Starting Game..." << std::endl;
//
//    Engine::Initialize();
//
//    while (Engine::IsRunning()) {
//        Engine::Update();
//
//        // Simple frame rate limiting
//        std::this_thread::sleep_for(std::chrono::milliseconds(16));
//    }
//
//    Engine::Shutdown();
//    std::cout << "Game ended." << std::endl;
//    return 0;
//}

#include "Engine.h"
#include "GameManager.h"
#include <iostream>
#include <thread>
#include <chrono>
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>

int main() {
    std::cout << "Starting Game..." << std::endl;

    GameManager gameManager;

    Engine::Initialize();
    //GameManager::Initialize();
    gameManager.Initialize();

    while (Engine::IsRunning()) {
        Engine::Update();
        //GameManager::Update();
        gameManager.Update();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    //GameManager::Shutdown();
    gameManager.Shutdown();
    Engine::Shutdown();
    std::cout << "Game ended." << std::endl;
    return 0;
}