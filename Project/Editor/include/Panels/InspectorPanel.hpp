#pragma once

#include "EditorPanel.hpp"

/**
 * @brief Inspector panel for viewing and editing properties of selected objects.
 * 
 * This panel displays detailed information and editable properties for the currently
 * selected entity or object, similar to Unity's Inspector window.
 */
class InspectorPanel : public EditorPanel {
public:
    InspectorPanel();
    virtual ~InspectorPanel() = default;

    /**
     * @brief Render the inspector panel's ImGui content.
     */
    void OnImGuiRender() override;

    /**
     * @brief Set the entity to inspect.
     * @param entityId The ID of the entity to show in the inspector.
     */
    void SetSelectedEntity(int entityId) { m_SelectedEntity = entityId; }

private:
    void DrawTransformComponent();
    void DrawMeshRendererComponent();
    void DrawCameraComponent();
    
    int m_SelectedEntity = -1;
    
    // Example component data - in a real implementation, this would come from the ECS
    float m_Position[3] = {0.0f, 0.0f, 0.0f};
    float m_Rotation[3] = {0.0f, 0.0f, 0.0f};
    float m_Scale[3] = {1.0f, 1.0f, 1.0f};
    float m_CameraFOV = 60.0f;
    float m_CameraNear = 0.1f;
    float m_CameraFar = 1000.0f;
};