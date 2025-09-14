#include "Panels/InspectorPanel.hpp"
#include "imgui.h"

InspectorPanel::InspectorPanel() 
    : EditorPanel("Inspector", true) {
}

void InspectorPanel::OnImGuiRender() {
    if (ImGui::Begin(m_Name.c_str(), &m_IsOpen)) {
        if (m_SelectedEntity == -1) {
            ImGui::Text("No object selected");
            ImGui::Text("Select an object in the Scene Hierarchy to view its properties");
        } else {
            ImGui::Text("Entity ID: %d", m_SelectedEntity);
            ImGui::Separator();

            // Entity name field
            static char entityName[128] = "Selected Entity";
            ImGui::InputText("Name", entityName, sizeof(entityName));
            
            // Active checkbox
            static bool isActive = true;
            ImGui::Checkbox("Active", &isActive);
            
            ImGui::Separator();

            // Transform Component
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                DrawTransformComponent();
            }

            // Mesh Renderer Component (example)
            if (m_SelectedEntity == 3 || m_SelectedEntity == 4) { // Player related entities
                if (ImGui::CollapsingHeader("Mesh Renderer")) {
                    DrawMeshRendererComponent();
                }
            }

            // Camera Component (only for camera entity)
            if (m_SelectedEntity == 1) { // Main Camera
                if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                    DrawCameraComponent();
                }
            }

            ImGui::Separator();

            // Add Component button
            if (ImGui::Button("Add Component")) {
                ImGui::OpenPopup("AddComponentPopup");
            }

            if (ImGui::BeginPopup("AddComponentPopup")) {
                if (ImGui::MenuItem("Mesh Renderer")) {
                    // TODO: Add mesh renderer component
                }
                if (ImGui::MenuItem("Box Collider")) {
                    // TODO: Add box collider component
                }
                if (ImGui::MenuItem("Rigidbody")) {
                    // TODO: Add rigidbody component
                }
                ImGui::EndPopup();
            }
        }
    }
    ImGui::End();
}

void InspectorPanel::DrawTransformComponent() {
    ImGui::PushID("Transform");
    
    ImGui::Text("Position");
    ImGui::DragFloat3("##Position", m_Position, 0.1f, -FLT_MAX, FLT_MAX, "%.3f");
    
    ImGui::Text("Rotation");
    ImGui::DragFloat3("##Rotation", m_Rotation, 1.0f, -180.0f, 180.0f, "%.1f°");
    
    ImGui::Text("Scale");
    ImGui::DragFloat3("##Scale", m_Scale, 0.1f, 0.001f, FLT_MAX, "%.3f");
    
    if (ImGui::Button("Reset")) {
        m_Position[0] = m_Position[1] = m_Position[2] = 0.0f;
        m_Rotation[0] = m_Rotation[1] = m_Rotation[2] = 0.0f;
        m_Scale[0] = m_Scale[1] = m_Scale[2] = 1.0f;
    }
    
    ImGui::PopID();
}

void InspectorPanel::DrawMeshRendererComponent() {
    ImGui::PushID("MeshRenderer");
    
    static bool castShadows = true;
    static bool receiveShadows = true;
    static int materialSlot = 0;
    
    ImGui::Checkbox("Cast Shadows", &castShadows);
    ImGui::Checkbox("Receive Shadows", &receiveShadows);
    
    ImGui::Text("Materials");
    ImGui::Combo("Material", &materialSlot, "Default\0Stone\0Metal\0Wood\0");
    
    if (ImGui::Button("Change Mesh")) {
        // TODO: Open mesh selector
    }
    
    ImGui::PopID();
}

void InspectorPanel::DrawCameraComponent() {
    ImGui::PushID("Camera");
    
    static bool isMainCamera = true;
    static int projectionType = 0; // 0 = Perspective, 1 = Orthographic
    
    ImGui::Checkbox("Main Camera", &isMainCamera);
    ImGui::Combo("Projection", &projectionType, "Perspective\0Orthographic\0");
    
    if (projectionType == 0) { // Perspective
        ImGui::SliderFloat("Field of View", &m_CameraFOV, 1.0f, 179.0f, "%.1f°");
    }
    
    ImGui::DragFloat("Near Plane", &m_CameraNear, 0.01f, 0.001f, m_CameraFar - 0.001f, "%.3f");
    ImGui::DragFloat("Far Plane", &m_CameraFar, 1.0f, m_CameraNear + 0.001f, 10000.0f, "%.1f");
    
    static float backgroundColor[4] = {0.2f, 0.3f, 0.3f, 1.0f};
    ImGui::ColorEdit4("Background", backgroundColor);
    
    ImGui::PopID();
}