#include "Engine.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

static GLFWwindow* window = nullptr;
static bool running = false;

void Engine::Initialize() {
    std::cout << "Engine initializing..." << std::endl;

    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW!" << std::endl;
        return;
    }

    window = glfwCreateWindow(800, 600, "Test Window", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create window!" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD!" << std::endl;
        return;
    }

    running = true;
    std::cout << "Engine initialized successfully!" << std::endl;
}

void Engine::Update() {
    glfwPollEvents();
    if (glfwWindowShouldClose(window)) {
        running = false;
    }
}

void Engine::Shutdown() {
    glfwTerminate();
    running = false;
    std::cout << "Engine shut down." << std::endl;
}

bool Engine::IsRunning() {
    return running;
}