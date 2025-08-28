#include "EditorManager.h"
#include "Engine.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>

bool EditorManager::initialized = false;

void EditorManager::Initialize() {
    if (initialized) return;

    std::cout << "EditorManager initializing..." << std::endl;

    // Make sure we have a valid GLFW window
    GLFWwindow* window = Engine::window;
    if (!window) {
        std::cout << "[EditorManager] ERROR: No current GLFW context!" << std::endl;
        return;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    bool glfwInitResult = ImGui_ImplGlfw_InitForOpenGL(window, true);
    if (!glfwInitResult) {
        std::cout << "[EditorManager] ERROR: Failed to initialize ImGui GLFW!" << std::endl;
        return;
    }

    bool openglInitResult = ImGui_ImplOpenGL3_Init("#version 430");
    if (!openglInitResult) {
        std::cout << "[EditorManager] ERROR: Failed to initialize ImGui OpenGL3!" << std::endl;
        return;
    }

    initialized = true;
    std::cout << "EditorManager initialized!" << std::endl;
}

void EditorManager::Update() {
    if (!initialized) return;

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void EditorManager::Render() {
    if (!initialized) return;

    // Create a simple dockspace
    ImGui::DockSpaceOverViewport();

    // Example editor window
    ImGui::Begin("Engine Test");
    ImGui::Text("Engine is running!");
    if (ImGui::Button("Test Button")) {
        std::cout << "Button clicked!" << std::endl;
    }
    ImGui::End();

    // Show ImGui demo window
    ImGui::ShowDemoWindow();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorManager::Shutdown() {
    if (!initialized) return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    initialized = false;
    std::cout << "EditorManager shut down!" << std::endl;
}