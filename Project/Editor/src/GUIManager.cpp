#pragma once
#include "pch.h"
#include "GUIManager.hpp"
#include "imgui.h"
#include "imgui_internal.h"  // Required for DockBuilder functions
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "WindowManager.hpp"

// Include panel headers
#include "Panels/ScenePanel.hpp"
#include "Panels/SceneHierarchyPanel.hpp"
#include "Panels/InspectorPanel.hpp"
#include "Panels/ConsolePanel.hpp"

// Static member definitions
std::unique_ptr<PanelManager> GUIManager::s_PanelManager = nullptr;
bool GUIManager::s_DockspaceInitialized = false;

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
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport / Platform Windows
 //   io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
	//io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
    // Set up dark theme
    io.IniFilename = "imgui.ini";  // Save layout to imgui.ini
    ImGui::StyleColorsDark();

    // Make sure the context is current
    glfwMakeContextCurrent(window);
    
    // Initialize platform/renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
    
    // Note: Framebuffer creation is delayed until first render call
    // This ensures GLAD is properly initialized first

    // Initialize panel manager and default panels
    s_PanelManager = std::make_unique<PanelManager>();
    SetupDefaultPanels();

    std::cout << "[GUIManager] Initialized with panel-based architecture" << std::endl;
}


void GUIManager::Render() {
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create main dockspace
    CreateDockspace();

    // Render menu bar
    RenderMenuBar();

    // Render all open panels
    if (s_PanelManager) {
        s_PanelManager->RenderOpenPanels();
    }

    // Handle multi-viewport rendering
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        
        glfwMakeContextCurrent(backup_current_context);
    }

    // Render ImGui
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
    
    // Clean up panel manager
    s_PanelManager.reset();

    // Clean up ImGui resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    std::cout << "[GUIManager] Shutdown complete" << std::endl;
}

void GUIManager::SetupDefaultPanels() {
    if (!s_PanelManager) return;

    // Register core editor panels
    s_PanelManager->RegisterPanel(std::make_shared<SceneHierarchyPanel>());
    s_PanelManager->RegisterPanel(std::make_shared<InspectorPanel>());
    /*s_PanelManager->RegisterPanel(std::make_shared<ConsolePanel>());*/
    s_PanelManager->RegisterPanel(std::make_shared<ScenePanel>());

    std::cout << "[GUIManager] Default panels registered" << std::endl;
}

void GUIManager::CreateDockspace() {
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

    ImGui::End();
}

void GUIManager::RenderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                // TODO: New scene functionality
            }
            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
                // TODO: Open scene functionality
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                // TODO: Save scene functionality
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // TODO: Exit application
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                // TODO: Undo functionality
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
                // TODO: Redo functionality
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (s_PanelManager) {
                // Panel toggles
                for (const auto& panel : s_PanelManager->GetAllPanels()) {
                    bool isOpen = panel->IsOpen();
                    if (ImGui::MenuItem(panel->GetName().c_str(), nullptr, &isOpen)) {
                        panel->SetOpen(isOpen);
                    }
                }
                ImGui::Separator();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                // TODO: About dialog
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}