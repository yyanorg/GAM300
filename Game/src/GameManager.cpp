#include "GameManager.h"

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

// Define the static member
bool GameManager::initialized = false;

void GameManager::Initialize() {
    if (!initialized) {
        std::cout << "GameManager initialized!" << std::endl;

        //// Print current working directory for debug
        //std::cout << "Current working directory: "
        //    << std::filesystem::current_path().string()
        //    << std::endl;

        //// Try to open the test resource file
        //std::ifstream file("Resources/test.txt");
        //if (file.is_open()) {
        //    std::cout << "Found Resources/test.txt!" << std::endl;

        //    std::string line;
        //    while (std::getline(file, line)) {
        //        std::cout << ">> " << line << std::endl;
        //    }

        //    file.close();
        //}
        //else {
        //    std::cerr << "Could not open Resources/test.txt (file missing or path wrong)." << std::endl;
        //}

        initialized = true;
    }
}

void GameManager::Update() {
    // Game logic here
}

void GameManager::Shutdown() {
    if (initialized) {
        std::cout << "GameManager shut down!" << std::endl;
        initialized = false;
    }
}
