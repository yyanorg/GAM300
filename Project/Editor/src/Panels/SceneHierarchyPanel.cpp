#include "Panels/SceneHierarchyPanel.hpp"
#include "imgui.h"

SceneHierarchyPanel::SceneHierarchyPanel() 
    : EditorPanel("Scene Hierarchy", true) {
}

void SceneHierarchyPanel::OnImGuiRender() {
    if (ImGui::Begin(m_Name.c_str(), &m_IsOpen)) {
        ImGui::Text("Scene Objects:");
        ImGui::Separator();

        // Example scene objects - in a real implementation, this would iterate through actual scene data
        DrawEntityNode("Main Camera", 1);
        DrawEntityNode("Directional Light", 2);
        DrawEntityNode("Player", 3, true);
        
        // Child objects under Player
        if (ImGui::TreeNode("Player")) {
            DrawEntityNode("Player Model", 4);
            DrawEntityNode("Player Controller", 5);
            ImGui::TreePop();
        }
        
        DrawEntityNode("Environment", 6, true);
        if (ImGui::TreeNode("Environment")) {
            DrawEntityNode("Terrain", 7);
            DrawEntityNode("Sky", 8);
            DrawEntityNode("Water", 9);
            ImGui::TreePop();
        }

        ImGui::Separator();
        
        // Context menu for creating new objects
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Create Empty")) {
                // TODO: Create new empty entity
            }
            if (ImGui::MenuItem("Create Cube")) {
                // TODO: Create cube primitive
            }
            if (ImGui::MenuItem("Create Sphere")) {
                // TODO: Create sphere primitive
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(const std::string& entityName, int entityId, bool hasChildren) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    
    if (!hasChildren) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }
    
    if (m_SelectedEntity == entityId) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool opened = ImGui::TreeNodeEx((void*)(intptr_t)entityId, flags, "%s", entityName.c_str());
    
    if (ImGui::IsItemClicked()) {
        m_SelectedEntity = entityId;
    }

    // Context menu for individual entities
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Delete")) {
            // TODO: Delete entity
        }
        if (ImGui::MenuItem("Duplicate")) {
            // TODO: Duplicate entity
        }
        if (ImGui::MenuItem("Rename")) {
            // TODO: Rename entity
        }
        ImGui::EndPopup();
    }

    if (opened && hasChildren) {
        // Child nodes would be drawn here in a real implementation
        ImGui::TreePop();
    }
}