#pragma once
#include "pch.h"
#include "GUIManager.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "WindowManager.hpp"

void GUIManager::Initialize() {
    GLFWwindow* window = WindowManager::getWindow();
	if (!window) {
		std::cerr << "Error: GLFW window is null. Cannot initialize ImGui." << std::endl;
		return;
	}
    std::cout << "Window ptr GUI: " << WindowManager::getWindow() << std::endl;

    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    ImGui::StyleColorsDark();

    // Initialize platform/renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
}

void GUIManager::Render() {
    
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Here you can add custom ImGui windows for debugging or settings
    ImGui::Begin("Example Window");
    ImGui::Text("Hello, ImGui!");
    ImGui::End();

    // Render ImGui on top of the scene
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIManager::Exit() {
    // Clean up ImGui resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}