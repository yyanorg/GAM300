#include "EditorInputManager.hpp"
#include <algorithm>

// Static member definitions
EditorInputManager::MouseState EditorInputManager::s_MouseState;
EditorInputManager::KeyboardState EditorInputManager::s_KeyboardState;

void EditorInputManager::Update() {
    UpdateMouseState();
    UpdateKeyboardState();
}

void EditorInputManager::UpdateMouseState() {
    ImGuiIO& io = ImGui::GetIO();

    // Get current mouse position
    glm::vec2 currentPos(io.MousePos.x, io.MousePos.y);

    // Calculate mouse delta (same as the original working method)
    if (!s_MouseState.firstMouse) {
        s_MouseState.delta = currentPos - s_MouseState.lastPosition;
    } else {
        s_MouseState.delta = glm::vec2(0.0f);
        s_MouseState.firstMouse = false;
    }

    // Update positions
    s_MouseState.position = currentPos;
    s_MouseState.lastPosition = currentPos;

    // Update scroll
    s_MouseState.scrollDelta = io.MouseWheel;

    // Update mouse button states (pressed = just pressed this frame)
    bool currentLeft = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool currentMiddle = ImGui::IsMouseDown(ImGuiMouseButton_Middle);
    bool currentRight = ImGui::IsMouseDown(ImGuiMouseButton_Right);

    s_MouseState.leftPressed = currentLeft && !s_MouseState.leftDown;
    s_MouseState.middlePressed = currentMiddle && !s_MouseState.middleDown;
    s_MouseState.rightPressed = currentRight && !s_MouseState.rightDown;

    s_MouseState.leftDown = currentLeft;
    s_MouseState.middleDown = currentMiddle;
    s_MouseState.rightDown = currentRight;
}

void EditorInputManager::UpdateKeyboardState() {
    ImGuiIO& io = ImGui::GetIO();

    // Update modifier keys
    s_KeyboardState.altPressed = io.KeyAlt;
    s_KeyboardState.ctrlPressed = io.KeyCtrl;
    s_KeyboardState.shiftPressed = io.KeyShift;

    // Update editor shortcut keys (only register as pressed if just pressed this frame)
    s_KeyboardState.qKeyPressed = ImGui::IsKeyPressed(ImGuiKey_Q);  // Normal/panning mode
    s_KeyboardState.wKeyPressed = ImGui::IsKeyPressed(ImGuiKey_W);  // Translate
    s_KeyboardState.eKeyPressed = ImGui::IsKeyPressed(ImGuiKey_E);  // Rotate
    s_KeyboardState.rKeyPressed = ImGui::IsKeyPressed(ImGuiKey_R);  // Scale
}

const EditorInputManager::MouseState& EditorInputManager::GetMouseState() {
    return s_MouseState;
}

const EditorInputManager::KeyboardState& EditorInputManager::GetKeyboardState() {
    return s_KeyboardState;
}

bool EditorInputManager::IsWindowHovered(const char* windowName) {
    // Check if the specific window is hovered
    // For now, we'll use ImGui::IsWindowHovered() but this could be enhanced
    // to check specific window names if needed
    return ImGui::IsWindowHovered();
}

glm::vec2 EditorInputManager::GetMouseDelta(float sensitivity) {
    return s_MouseState.delta * sensitivity;
}

bool EditorInputManager::ShouldHandleCameraInput(const char* windowName) {
    // Only handle camera input if the specified window is hovered
    // Allow camera input unless we're typing in a text field
    return ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered();
}

bool EditorInputManager::IsGizmoShortcutPressed(int gizmoType) {
    switch (gizmoType) {
        case 0: return s_KeyboardState.qKeyPressed;  // Normal/panning mode
        case 1: return s_KeyboardState.wKeyPressed;  // Translate
        case 2: return s_KeyboardState.eKeyPressed;  // Rotate
        case 3: return s_KeyboardState.rKeyPressed;  // Scale
        default: return false;
    }
}

