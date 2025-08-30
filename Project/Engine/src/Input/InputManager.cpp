#include "pch.h"
#include "InputManager.hpp"

std::unordered_map<int, bool> InputManager::keyStates;
std::unordered_map<int, bool> InputManager::mouseButtonStates;
std::unordered_map<int, bool> InputManager::prevKeyStates;
std::unordered_map<int, bool> InputManager::prevMouseButtonStates;
double InputManager::mouseX = 0.0;
double InputManager::mouseY = 0.0;
double InputManager::scrollOffsetX = 0.0;
double InputManager::scrollOffsetY = 0.0;

void InputManager::Initialize(GLFWwindow* window)
{
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetCursorPosCallback(window, CursorPositionCallback);
	glfwSetScrollCallback(window, ScrollCallback);
}

void InputManager::Update()
{
	prevKeyStates = keyStates;
	prevMouseButtonStates = mouseButtonStates;

	WindowManager::PollEvents();
}

bool InputManager::GetKeyDown(int key)
{
	return keyStates[key] && !prevKeyStates[key];
}

bool InputManager::GetKey(int key)
{
	auto it = keyStates.find(key);
	return it != keyStates.end() && it->second;
}

bool InputManager::GetMouseButtonDown(int button)
{
	return mouseButtonStates[button] && !prevMouseButtonStates[button];
}

bool InputManager::GetMouseButton(int button)
{
	auto it = mouseButtonStates.find(button);
	return it != mouseButtonStates.end() && it->second;
}

double InputManager::GetMouseX()
{
	return mouseX;
}

double InputManager::GetMouseY()
{
	return mouseY;
}

bool InputManager::GetAnyKeyDown()
{
	for (const auto& keyState : keyStates) {
		if (GetKeyDown(keyState.first)) {
			return true;
		}
	}

	return false;
}

bool InputManager::GetAnyMouseButtonDown()
{
	for (const auto& mouseState : mouseButtonStates) {
		if (GetMouseButtonDown(mouseState.first)) {
			return true;
		}
	}

	return false;
}

bool InputManager::GetAnyInputDown()
{
	return GetAnyKeyDown() || GetAnyMouseButtonDown();
}
