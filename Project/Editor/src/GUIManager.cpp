#pragma once
#include "pch.h"
#include "GUIManager.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuizmo.h"
#include "WindowManager.hpp"
#include "EditorState.hpp"

// Include panel headers
#include "Panels/ScenePanel.hpp"
#include "Panels/GamePanel.hpp"
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
    
    // Initialize ImGuizmo for this frame
    ImGuizmo::BeginFrame();

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

    std::cout << "[GUIManager] Shutdown complete" << std::endl;
}

void GUIManager::SetupDefaultPanels() {
    assert(s_PanelManager != nullptr && "PanelManager must be initialized before setting up panels");

    // Register core editor panels
    auto sceneHierarchyPanel = std::make_shared<SceneHierarchyPanel>();
    assert(sceneHierarchyPanel != nullptr && "Failed to create SceneHierarchyPanel");
    s_PanelManager->RegisterPanel(sceneHierarchyPanel);
    
    auto inspectorPanel = std::make_shared<InspectorPanel>();
    assert(inspectorPanel != nullptr && "Failed to create InspectorPanel");
    s_PanelManager->RegisterPanel(inspectorPanel);
    
    auto consolePanel = std::make_shared<ConsolePanel>();
    assert(consolePanel != nullptr && "Failed to create ConsolePanel");
    s_PanelManager->RegisterPanel(consolePanel);
    
    auto scenePanel = std::make_shared<ScenePanel>();
    assert(scenePanel != nullptr && "Failed to create ScenePanel");
    s_PanelManager->RegisterPanel(scenePanel);
    
    auto gamePanel = std::make_shared<GamePanel>();
    assert(gamePanel != nullptr && "Failed to create GamePanel");
    s_PanelManager->RegisterPanel(gamePanel);

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

        // Initialize default docking layout only once
        if (!s_DockspaceInitialized) {
            s_DockspaceInitialized = true;

            // Clear any existing layout
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

            // Split the dockspace into sections
            ImGuiID dock_left, dock_right, dock_center, dock_bottom;
            ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, &dock_left, &dock_center);
            ImGui::DockBuilderSplitNode(dock_center, ImGuiDir_Right, 0.25f, &dock_right, &dock_center);
            ImGui::DockBuilderSplitNode(dock_center, ImGuiDir_Down, 0.3f, &dock_bottom, &dock_center);

            // Dock windows to specific areas
            ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_left);
            ImGui::DockBuilderDockWindow("Inspector", dock_right);
            ImGui::DockBuilderDockWindow("Console", dock_bottom);
            ImGui::DockBuilderDockWindow("Scene", dock_center);
            ImGui::DockBuilderDockWindow("Game", dock_center);  // Game panel in center with Scene

            ImGui::DockBuilderFinish(dockspace_id);
        }
    }

    ImGui::End();
}

void GUIManager::RenderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        // Left-side menus first
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

        // Center the play/stop buttons like Unity
        float menuBarWidth = ImGui::GetWindowWidth();
        float buttonWidth = 180.0f; // Updated width for thicker play/stop buttons + spacing
        float centerPos = (menuBarWidth - buttonWidth) * 0.5f;

        // Calculate current cursor position and add spacing to center the buttons
        float currentPos = ImGui::GetCursorPosX();
        if (centerPos > currentPos) {
            ImGui::SetCursorPosX(centerPos);
        }

        // Play/Pause controls in the center
        EditorState& editorState = EditorState::GetInstance();

        // Make buttons thicker by pushing style variables
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 8.0f));  // Thicker padding

        // Play button
        if (editorState.IsEditMode() || editorState.IsPaused()) {
            if (ImGui::Button("▶ Play", ImVec2(80.0f, 0.0f))) {  // Fixed width for consistency
                editorState.Play();
                // Auto-focus the Game panel when play is pressed
                if (s_PanelManager) {
                    auto gamePanel = s_PanelManager->GetPanel("Game");
                    if (gamePanel) {
                        gamePanel->SetOpen(true);
                        // Focus the Game window by setting it as the next window to focus
                        ImGui::SetWindowFocus("Game");
                    }
                }
            }
        } else {
            // Show pause button when playing
            if (ImGui::Button("⏸ Pause", ImVec2(80.0f, 0.0f))) {
                editorState.Pause();
            }
        }

        ImGui::SameLine();

        // Stop button (always enabled)
        if (ImGui::Button("⏹ Stop", ImVec2(70.0f, 0.0f))) {  // Fixed width for consistency
            editorState.Stop();
            // Auto-switch to Scene panel when stopping
            if (s_PanelManager) {
                auto scenePanel = s_PanelManager->GetPanel("Scene");
                if (scenePanel) {
                    scenePanel->SetOpen(true);
                    // Focus the Scene window
                    ImGui::SetWindowFocus("Scene");
                }
            }
        }

        // Pop the style variable
        ImGui::PopStyleVar();

        ImGui::SameLine();

        // State indicator
        const char* stateText = editorState.IsEditMode() ? "EDIT" :
                               editorState.IsPlayMode() ? "PLAY" :
                               "PAUSED";
        ImGui::TextColored(editorState.IsPlayMode() ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f) :
                          editorState.IsPaused() ? ImVec4(1.0f, 0.6f, 0.0f, 1.0f) :
                          ImVec4(0.7f, 0.7f, 0.7f, 1.0f), " | %s", stateText);

        ImGui::EndMainMenuBar();
    }
}

void GUIManager::CreateEditorTheme() {
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    // Set up dark theme
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("Resources/Inter.ttf", 18.0f);
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale
    io.ConfigDpiScaleFonts = true;          // This will scale fonts but _NOT_ scale sizes/padding
    io.ConfigDpiScaleViewports = true;

    ImVec4* colors = style.Colors;

    // General Background colors
    colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);     // Dark gray
    colors[ImGuiCol_ChildBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);      // Slightly lighter gray
    colors[ImGuiCol_PopupBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);      // Popup background

    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.28f, 0.28f, 0.28f, 1.0f);       // Medium gray border
    colors[ImGuiCol_BorderShadow] = ImVec4(0.10f, 0.10f, 0.10f, 0.5f); // Subtle shadow

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.0f);         // White text
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.0f); // Gray text for disabled items

    // Headers
    colors[ImGuiCol_Header] = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);       // Header background
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.0f); // Hovered header
    colors[ImGuiCol_HeaderActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.0f); // Active header

    // Buttons
    colors[ImGuiCol_Button] = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);       // Button background
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.0f); // Hovered button
    colors[ImGuiCol_ButtonActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.0f); // Active button

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);          // Tab background
    colors[ImGuiCol_TabHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);   // Hovered tab
    colors[ImGuiCol_TabActive] = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);    // Active tab
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f); // Unfocused tab
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.0f); // Unfocused active tab

    // Title Bar (for windows)
    colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);      // Title background
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f); // Active title background
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.16f, 0.16f, 1.0f); // Collapsed title

    // Menu Bar
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);    // Menu bar background

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);  // Scrollbar background
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.28f, 0.28f, 0.28f, 1.0f); // Scrollbar grab
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.33f, 0.33f, 0.33f, 1.0f); // Hovered grab
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.36f, 0.36f, 0.36f, 1.0f); // Active grab

    // Slider
    colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.28f, 0.28f, 1.0f);   // Slider grab
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.0f); // Active slider

    // Rounding settings for a polished look
    style.WindowRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
}