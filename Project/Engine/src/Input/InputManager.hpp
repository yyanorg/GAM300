#pragma once

#include <unordered_map>
#include <WindowManager.hpp>

// #ifdef ENGINE_EXPORTS
// #define ENGINE_API __declspec(dllexport)
// #else
// #define ENGINE_API __declspec(dllimport)
// #endif

class InputManager {
public:
	static void Initialize(GLFWwindow* window);
	static void Update();

	static bool GetKeyDown(int key);
	static bool GetKey(int key);

	static bool GetMouseButtonDown(int button);
	static bool GetMouseButton(int button);

	static double GetMouseX();
	static double GetMouseY();

	static bool GetAnyKeyDown();
	static bool GetAnyMouseButtonDown();
	static bool GetAnyInputDown();

private:
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		window, scancode, mods;
		keyStates[key] = (action != GLFW_RELEASE);
		//ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	}

	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		window, mods;
		mouseButtonStates[button] = (action != GLFW_RELEASE);
		//ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	}

	static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
		window;
		mouseX = xpos;
		mouseY = ypos;
		//ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
	}

	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		window;
		scrollOffsetX += xoffset;
		scrollOffsetY += yoffset;
		//ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	}

	static std::unordered_map<int, bool> keyStates;
	static std::unordered_map<int, bool> mouseButtonStates;

	static std::unordered_map<int, bool> prevKeyStates;
	static std::unordered_map<int, bool> prevMouseButtonStates;

	static double mouseX;
	static double mouseY;
	static double scrollOffsetX;
	static double scrollOffsetY;
};