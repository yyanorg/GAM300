#pragma once

#include "EditorPanel.hpp"

/**
 * @brief Scene Hierarchy panel showing the structure of objects in the current scene.
 * 
 * This panel displays a tree view of all entities and objects in the scene,
 * similar to Unity's Hierarchy window.
 */
class SceneHierarchyPanel : public EditorPanel {
public:
    SceneHierarchyPanel();
    virtual ~SceneHierarchyPanel() = default;

    /**
     * @brief Render the scene hierarchy panel's ImGui content.
     */
    void OnImGuiRender() override;

private:
    void DrawEntityNode(const std::string& entityName, int entityId, bool hasChildren = false);
    
    int m_SelectedEntity = -1;
};