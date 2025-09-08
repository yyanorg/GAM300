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

    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
    io.IniFilename = "imgui.ini";  // Save layout to imgui.ini
    ImGui::StyleColorsDark();

    // Make sure the context is current
    glfwMakeContextCurrent(window);
    
    // Initialize platform/renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
    
    // Note: Framebuffer creation is delayed until first render call
    // This ensures GLAD is properly initialized first
}


void GUIManager::Render() {
    
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create a fullscreen dockspace
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    // Create the docking space
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }

    // Menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            ImGui::MenuItem("New", "Ctrl+N");
            ImGui::MenuItem("Open", "Ctrl+O");
            ImGui::MenuItem("Save", "Ctrl+S");
            ImGui::Separator();
            ImGui::MenuItem("Exit");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Console");
            ImGui::MenuItem("Inspector");
            ImGui::MenuItem("Scene Hierarchy");
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End();

    ImGui::Begin("Scene");
    
    // Get the content region size
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    int sceneViewWidth = (int)viewportPanelSize.x;
    int sceneViewHeight = (int)viewportPanelSize.y;
    
    // Ensure minimum size
    if (sceneViewWidth < 100) sceneViewWidth = 100;
    if (sceneViewHeight < 100) sceneViewHeight = 100;
    
    // Render 3D scene using WindowManager
    WindowManager::BeginSceneRender(sceneViewWidth, sceneViewHeight);
    WindowManager::RenderScene();
    WindowManager::EndSceneRender();
    
    // Get the texture from WindowManager and display it
    unsigned int sceneTexture = WindowManager::GetSceneTexture();
    if (sceneTexture != 0) {
        ImGui::Image(
            (void*)(intptr_t)sceneTexture,
            ImVec2((float)sceneViewWidth, (float)sceneViewHeight),
            ImVec2(0, 1), ImVec2(1, 0)  // Flip Y coordinate for OpenGL
        );
    } else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Scene View - Framebuffer not ready");
        ImGui::Text("Size: %dx%d", sceneViewWidth, sceneViewHeight);
    }
    
    ImGui::End();

    // Other dockable windows
    ImGui::Begin("Console");
    ImGui::Text("This is the console window hopefully soon");
    ImGui::End();

    ImGui::Begin("Inspector");
    ImGui::Text("Inspector Window");
    ImGui::Text("Transform");
    static float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    ImGui::SliderFloat("Position X", &posX, -100.0f, 100.0f);
    ImGui::SliderFloat("Position Y", &posY, -100.0f, 100.0f);
    ImGui::SliderFloat("Position Z", &posZ, -100.0f, 100.0f);
    ImGui::End();

    ImGui::Begin("Scene Hierarchy");
    ImGui::Text("Scene Objects:");
	if (ImGui::TreeNode("TESTING NOT DONE")) {
		ImGui::Text("BABY");
		ImGui::TreePop();
	}
    ImGui::End();

    ImGui::Begin("Asset Browser");
    ImGui::Text("Assets:");
    ImGui::Button("texture1.png");
    ImGui::Button("model.obj");
    ImGui::Button("shader.glsl");
    ImGui::End();

    // Render ImGui on top of the scene
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Required for multi-viewport support:
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void GUIManager::Exit() {
    // WindowManager handles framebuffer cleanup
    WindowManager::DeleteSceneFramebuffer();
    
    // Clean up ImGui resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}