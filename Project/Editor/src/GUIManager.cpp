#pragma once
#include "pch.h"
#include "GUIManager.hpp"
#include "imgui.h"
#include "imgui_internal.h"  // Required for DockBuilder functions
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuizmo.h"
#include "WindowManager.hpp"
#include "EditorState.hpp"

// Include panel headers
#include "Panels/ScenePanel.hpp"
#include "Panels/SceneHierarchyPanel.hpp"
#include "Panels/InspectorPanel.hpp"
#include "Panels/ConsolePanel.hpp"
#include "Panels/GamePanel.hpp"
#include "Panels/PlayControlPanel.hpp"
#include "Panels/PerformancePanel.hpp"

// Static member definitions
std::unique_ptr<PanelManager> GUIManager::s_PanelManager = nullptr;
bool GUIManager::s_DockspaceInitialized = false;

void GUIManager::Initialize() {
	GLFWwindow* window = WindowManager::getWindow();
	assert(window != nullptr && "GLFW window must be valid before initializing ImGui");
	
	// Make sure the context is current
	glfwMakeContextCurrent(window);

	// ImGui initialization
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport / Platform Windows
 //   io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
	//io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
	
	CreateEditorTheme();

	// Initialize platform/renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450");
	
	// Note: Framebuffer creation is delayed until first render call
	// This ensures GLAD is properly initialized first

	// Initialize panel manager and default panels
	s_PanelManager = std::make_unique<PanelManager>();
	assert(s_PanelManager != nullptr && "Failed to create PanelManager");
	
	SetupDefaultPanels();

	std::cout << "[GUIManager] Initialized with panel-based architecture" << std::endl;
}


void GUIManager::Render() {
	assert(s_PanelManager != nullptr && "PanelManager must be initialized before rendering");
	
	// Start ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	// Initialize ImGuizmo for this frame
	ImGuizmo::BeginFrame();

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
		assert(backup_current_context != nullptr && "No current GLFW context");
		
		glfwMakeContextCurrent(backup_current_context);
	}

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
	
	// Clean up panel manager
	s_PanelManager.reset();

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

	auto playControlPanel = std::make_shared<PlayControlPanel>();
	assert(playControlPanel != nullptr && "Failed to create PlayControlPanel");
	s_PanelManager->RegisterPanel(playControlPanel);

	auto performancePanel = std::make_shared<PerformancePanel>();
	assert(performancePanel != nullptr && "Failed to create PerformancePanel");
	s_PanelManager->RegisterPanel(performancePanel);

	std::cout << "[GUIManager] Default panels registered" << std::endl;
}

void GUIManager::CreateDockspace() {
	// Create a fullscreen dockspace, accounting for the play controls toolbar
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	float menuBarHeight = ImGui::GetFrameHeight();
	float toolbarHeight = ImGui::GetFrameHeight() + 16.0f; // Match toolbar height from PlayControlPanel

	// Position dockspace below menu bar and toolbar
	ImVec2 dockspacePos = ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight + toolbarHeight);
	ImVec2 dockspaceSize = ImVec2(viewport->Size.x, viewport->Size.y - menuBarHeight - toolbarHeight);

	ImGui::SetNextWindowPos(dockspacePos);
	ImGui::SetNextWindowSize(dockspaceSize);
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

		// Check if we need to initialize the default layout
		// Only do this if no docking data exists for this dockspace
		if (!s_DockspaceInitialized) {
			s_DockspaceInitialized = true;

			// Check if the dockspace already has nodes (from saved layout)
			ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspace_id);
			if (!node || !node->IsSplitNode()) {
				// No saved layout exists, create default layout
				ImGui::DockBuilderRemoveNode(dockspace_id);
				ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
				ImGui::DockBuilderSetNodeSize(dockspace_id, dockspaceSize);

				// Create the splits
				ImGuiID dock_left, dock_right, dock_down;
				ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, &dock_left, &dock_right);
				ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Down, 0.3f, &dock_down, &dock_right);

				// Dock the panels to default positions
				ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_left);
				ImGui::DockBuilderDockWindow("Scene Panel", dock_right);
				ImGui::DockBuilderDockWindow("Game", dock_right);
				ImGui::DockBuilderDockWindow("Inspector", dock_left);
				ImGui::DockBuilderDockWindow("Console", dock_down);
				ImGui::DockBuilderDockWindow("Performance", dock_down);

				// Ensure all panels are open by default so they get saved to the layout
				if (s_PanelManager) {
					auto gamePanel = s_PanelManager->GetPanel("Game");
					auto scenePanel = s_PanelManager->GetPanel("Scene Panel");
					auto hierarchyPanel = s_PanelManager->GetPanel("Scene Hierarchy");
					auto inspectorPanel = s_PanelManager->GetPanel("Inspector");
					auto consolePanel = s_PanelManager->GetPanel("Console");
					auto performancePanel = s_PanelManager->GetPanel("Performance");

					if (gamePanel) gamePanel->SetOpen(true);
					if (scenePanel) scenePanel->SetOpen(true);
					if (hierarchyPanel) hierarchyPanel->SetOpen(true);
					if (inspectorPanel) inspectorPanel->SetOpen(true);
					if (consolePanel) consolePanel->SetOpen(true);
					if (performancePanel) performancePanel->SetOpen(true);
				}

				ImGui::DockBuilderFinish(dockspace_id);
			}
		}
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