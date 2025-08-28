#include "pch.h"
#include <glad/glad.h>
#include "Engine.h"
#include "WindowManager.h"
#include "GraphicsManager.h"

static bool initialized = false;
static bool running = false;
static bool renderToFBO = false;

void Engine::Initialize() {
    std::cout << "[Engine] Initializing..." << std::endl;

    if (!WindowManager::Initialize(1200, 800, "Engine")) {
        std::cout << "[Engine] Window initialization failed!" << std::endl;
        return;
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "[Engine] GLAD initialization failed!" << std::endl;
        return;
    }

    std::cout << "[Engine] OpenGL " << glGetString(GL_VERSION) << std::endl;

    GraphicsManager::Initialize();

    initialized = true;
    running = true;
    std::cout << "[Engine] Ready" << std::endl;
}

void Engine::Update() {
    if (!running) return;

    WindowManager::PollEvents();

    if (WindowManager::ShouldClose()) {
        running = false;
    }

    // Handle ESC key for game mode
    GLFWwindow* window = WindowManager::GetWindow();
    if (window && glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        running = false;
    }
}

void Engine::StartDraw() {
    if (!initialized) return;

    glViewport(0, 0, WindowManager::GetWindowWidth(), WindowManager::GetWindowHeight());
}

void Engine::Draw() {
    if (!initialized) return;

    GraphicsManager::DrawTestCube();
}

void Engine::EndDraw() {
    if (!initialized) return;

    //if (renderToFBO) {
    //    GraphicsManager::UnbindFBO();
    //}
    //else {
    //    // Game mode - swap buffers directly
    //    WindowManager::SwapBuffers();
    //}

    WindowManager::SwapBuffers();
}

void Engine::SetRenderToFramebuffer(bool enabled) {
    renderToFBO = enabled;
}

unsigned int Engine::GetFramebufferTexture() {
    return GraphicsManager::GetFBOTexture();
}

bool Engine::IsRunning() {
    return running;
}

void Engine::Shutdown() {
    GraphicsManager::Shutdown();
    WindowManager::Shutdown();
    running = false;
    std::cout << "[Engine] Shutdown complete" << std::endl;
}