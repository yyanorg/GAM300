#pragma once

#include "EditorPanel.hpp"
#include "EditorCamera.hpp"
#include "imgui.h"
#include "ImGuizmo.h"

/**
 * @brief Scene editing panel with ImGuizmo integration.
 * 
 * This panel provides scene editing capabilities with gizmos for 
 * transforming objects, grid visualization, and other scene editing tools.
 */
class ScenePanel : public EditorPanel {
public:
    ScenePanel();
    virtual ~ScenePanel() = default;

    /**
     * @brief Render the scene panel's ImGui content with ImGuizmo tools.
     */
    void OnImGuiRender() override;

private:
    // ImGuizmo state
    ImGuizmo::OPERATION m_GizmoOperation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE m_GizmoMode = ImGuizmo::WORLD;

    // Editor modes
    bool m_IsNormalPanMode = false;  // Q key mode - no gizmos, LMB panning
    
    // Editor camera for this panel
    EditorCamera m_EditorCamera;

    // Input tracking for camera (reverted from EditorInputManager for working orbit)
    glm::vec2 m_LastMousePos;
    bool m_FirstMouse = true;

    // Matrix storage for ImGuizmo
    float m_IdentityMatrix[16];
    
    void InitializeMatrices();
    void HandleKeyboardInput();
    void HandleCameraInput();
    void HandleEntitySelection();
    void RenderGizmoControls();
    void RenderSceneWithEditorCamera(int width, int height);
    void HandleImGuizmoInChildWindow(float sceneWidth, float sceneHeight);

    // Helper functions
    void Mat4ToFloatArray(const glm::mat4& mat, float* arr);
};