#include "pch.h"
#include "WindowManager.h"
#include <iostream>

GLFWwindow* WindowManager::window = nullptr;
int WindowManager::width = 0;
int WindowManager::height = 0;
bool WindowManager::focused = true;

bool WindowManager::Initialize(int w, int h, const char* title) {
    std::cout << "[WindowManager] Initializing " << w << "x" << h << std::endl;

    if (!glfwInit()) {
        std::cout << "[WindowManager] GLFW initialization failed!" << std::endl;
        return false;
    }

    glfwSetErrorCallback(ErrorCallback);

    // OpenGL hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    window = glfwCreateWindow(w, h, title, nullptr, nullptr);
    if (!window) {
        std::cout << "[WindowManager] Window creation failed!" << std::endl;
        glfwTerminate();
        return false;
    }

    width = w;
    height = h;

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetWindowFocusCallback(window, WindowFocusCallback);
    glfwSwapInterval(1); // VSync

    std::cout << "[WindowManager] Window created successfully" << std::endl;
    return true;
}

void WindowManager::Shutdown() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
    std::cout << "[WindowManager] Shutdown complete" << std::endl;
}

GLFWwindow* WindowManager::GetWindow() {
    return window;
}

bool WindowManager::ShouldClose() {
    return window ? glfwWindowShouldClose(window) : true;
}

void WindowManager::SetShouldClose() {
    if (window) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void WindowManager::PollEvents() {
    glfwPollEvents();
}

void WindowManager::SwapBuffers() {
    if (window) {
        glfwSwapBuffers(window);
    }
}

int WindowManager::GetWindowWidth() {
    return width;
}

int WindowManager::GetWindowHeight() {
    return height;
}

void WindowManager::SetWindowTitle(const char* title) {
    if (window) {
        glfwSetWindowTitle(window, title);
    }
}

bool WindowManager::IsFocused() {
    return focused;
}

void WindowManager::ErrorCallback(int error, const char* description) {
    std::cout << "[WindowManager] GLFW Error " << error << ": " << description << std::endl;
}

void WindowManager::FramebufferSizeCallback(GLFWwindow* win, int w, int h) {
    width = w;
    height = h;
    glViewport(0, 0, w, h);
    std::cout << "[WindowManager] Window resized: " << w << "x" << h << std::endl;
}

void WindowManager::WindowFocusCallback(GLFWwindow* win, int focused) {
    WindowManager::focused = (focused != 0);
}