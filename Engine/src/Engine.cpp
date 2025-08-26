#include "Engine.h"
#include "TestScene.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// Static member definitions
GLFWwindow* Engine::window = nullptr;
bool Engine::running = false;

void Engine::Initialize() {
    std::cout << "Engine initializing..." << std::endl;

    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW!" << std::endl;
        return;
    }

    // Set OpenGL version hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "Engine Test Window", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD!" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return;
    }

    // Set viewport
    glViewport(0, 0, 800, 600);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    running = true;
    std::cout << "Engine initialized successfully!" << std::endl;
    std::cout << std::endl;

    // Run all library tests
    TestScene::RunAllTests();
}

void Engine::Update() {
    if (!window || !running) {
        return;
    }

    // Poll for and process events
    glfwPollEvents();

    // Check if window should close
    if (glfwWindowShouldClose(window)) {
        running = false;
        return;
    }

    // Clear the screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Swap front and back buffers
    glfwSwapBuffers(window);
}

void Engine::Shutdown() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
    running = false;
    std::cout << "Engine shut down." << std::endl;
}

bool Engine::IsRunning() {
    return running;
}