// Forward declaration for WindowManager
class WindowManager;

#pragma once

#include <memory>
#include "Panels/PanelManager.hpp"

/**
 * @brief Main GUI management class for the editor.
 * 
 * The GUIManager serves as the editor "layer" that handles:
 * - Global ImGui setup and teardown
 * - Central dockspace layout creation
 * - Panel registration and management delegation
 * - Multi-viewport rendering coordination
 */
class GUIManager {
public:
	/**
	 * @brief Initialize the GUI system.
	 * Sets up ImGui context, configures docking and viewports, and registers default panels.
	 */
	static void Initialize();

	/**
	 * @brief Render the GUI system.
	 * Creates the main dockspace and renders all open panels.
	 */
	static void Render();

	/**
	 * @brief Clean up and exit the GUI system.
	 * Shuts down ImGui and cleans up resources.
	 */
	static void Exit();

	/**
	 * @brief Get the panel manager instance.
	 * @return Reference to the panel manager for external panel operations.
	 */
	static PanelManager& GetPanelManager() { return *s_PanelManager; }

private:
	/**
	 * @brief Set up the default editor panels.
	 * Registers and configures the core editor panels (Hierarchy, Inspector, Console, etc.).
	 */
	static void SetupDefaultPanels();

	/**
	 * @brief Create and configure the main editor dockspace.
	 * Sets up a Unity-like layout with docked windows.
	 */
	static void CreateDockspace();

	/**
	 * @brief Render the main menu bar.
	 * Provides access to panel toggles and editor functions.
	 */
	static void RenderMenuBar();

	static std::unique_ptr<PanelManager> s_PanelManager;
	static bool s_DockspaceInitialized;
};