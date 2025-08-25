//#pragma once
//
//#include "Engine.h"
//#include <iostream>
//#include <fstream>
//#include <string>
//#include <filesystem>
//
//class GameManager {
//public:
//    //static void Initialize();
//    //static void Update();
//    //static void Shutdown();
//
//    void Initialize() {
//        if (!initialized) {
//            std::cout << "GameManager initialized!" << std::endl;
//
//            std::cout << "Current working directory: "
//                << std::filesystem::current_path().string()
//                << std::endl;
//
//            // Try to open the test resource file
//            std::ifstream file("Resources/test.txt");
//            if (file.is_open()) {
//                std::cout << "Found Resources/test.txt!" << std::endl;
//
//                std::string line;
//                while (std::getline(file, line)) {
//                    std::cout << ">> " << line << std::endl;
//                }
//
//                file.close();
//            }
//            else {
//                std::cerr << "Could not open Resources/test.txt (file missing or path wrong)." << std::endl;
//            }
//
//            initialized = true;
//        }
//    }
//
//    void Update() {
//        // Game logic here
//    }
//
//    void Shutdown() {
//        if (initialized) {
//            std::cout << "GameManager shut down!" << std::endl;
//            initialized = false;
//        }
//    }
//
//private:
//    bool initialized = false;
//};

#pragma once

class GameManager {
public:
    // Static methods for global access
    static void Initialize();
    static void Update();
    static void Shutdown();

private:
    // Static member to track initialization
    static bool initialized;
};
