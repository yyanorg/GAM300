//#include "GameManager.h"
//#include <iostream>
//#include <fstream>
//#include <string>
//
//bool GameManager::initialized = false;
//
//void GameManager::Initialize() {
//    if (!initialized) {
//        std::cout << "GameManager initialized!" << std::endl;
//
//        // Try to open the test resource file
//        std::ifstream file("Resources/test.txt");
//        if (file.is_open()) {
//            std::cout << "Found Resources/test.txt!" << std::endl;
//
//            std::string line;
//            while (std::getline(file, line)) {
//                std::cout << ">> " << line << std::endl;
//            }
//
//            file.close();
//        }
//        else {
//            std::cerr << "Could not open Resources/test.txt (file missing or path wrong)." << std::endl;
//        }
//
//        initialized = true;
//    }
//}
//
//void GameManager::Update() {
//    // Game logic here
//}
//
//void GameManager::Shutdown() {
//    if (initialized) {
//        std::cout << "GameManager shut down!" << std::endl;
//        initialized = false;
//    }
//}
