#include "Panels/ScenePanel.hpp"
#include "EditorInputManager.hpp"
#include "Graphics/GraphicsManager.hpp"
#include "Graphics/SceneRenderer.hpp"
#include "RaycastUtil.hpp"
#include "imgui.h"
#include "ImGuizmo.h"
#include "EditorState.hpp"
#include "GUIManager.hpp"
#include <cstring>
#include <cmath>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

// Don't include Graphics headers here due to OpenGL conflicts
// We'll use RaycastUtil to get entity transforms instead

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ScenePanel::ScenePanel()
    : EditorPanel("Scene", true), editorCamera(glm::vec3(0.0f, 0.0f, 0.0f), 5.0f) {
    InitializeMatrices();
}

void ScenePanel::InitializeMatrices() {
    // Initialize identity matrix for ImGuizmo
    memset(identityMatrix, 0, sizeof(identityMatrix));
    identityMatrix[0] = identityMatrix[5] = identityMatrix[10] = identityMatrix[15] = 1.0f;
}

void ScenePanel::Mat4ToFloatArray(const glm::mat4& mat, float* arr) {
    const float* source = glm::value_ptr(mat);
    for (int i = 0; i < 16; ++i) {
        arr[i] = source[i];
    }
}


void ScenePanel::HandleKeyboardInput() {
    // Check keyboard input regardless of camera input conditions
    if (EditorInputManager::IsGizmoShortcutPressed(0)) {
        // Q key - Normal pan mode (no gizmos, left click panning)
        isNormalPanMode = true;
        std::cout << "[ScenePanel] Q pressed - Normal/Pan mode (gizmos hidden, LMB panning)" << std::endl;
    }
    if (EditorInputManager::IsGizmoShortcutPressed(1)) {
        isNormalPanMode = false;
        gizmoOperation = ImGuizmo::TRANSLATE;
        std::cout << "[ScenePanel] W pressed - Translate mode" << std::endl;
    }
    if (EditorInputManager::IsGizmoShortcutPressed(2)) {
        isNormalPanMode = false;
        gizmoOperation = ImGuizmo::ROTATE;
        std::cout << "[ScenePanel] E pressed - Rotate mode" << std::endl;
    }
    if (EditorInputManager::IsGizmoShortcutPressed(3)) {
        isNormalPanMode = false;
        gizmoOperation = ImGuizmo::SCALE;
        std::cout << "[ScenePanel] R pressed - Scale mode" << std::endl;
    }
}

void ScenePanel::HandleCameraInput() {
    // Hover check is now handled by the caller

    // Get current mouse position (revert to original working method)
    ImGuiIO& io = ImGui::GetIO();
    glm::vec2 currentMousePos = glm::vec2(io.MousePos.x, io.MousePos.y);

    // Calculate mouse delta (original working logic)
    glm::vec2 mouseDelta(0.0f);
    if (!firstMouse) {
        mouseDelta = currentMousePos - lastMousePos;
    } else {
        firstMouse = false;
    }
    lastMousePos = currentMousePos;

    // Get input states
    bool isAltPressed = io.KeyAlt;
    bool isLeftMousePressed = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool isMiddleMousePressed = ImGui::IsMouseDown(ImGuiMouseButton_Middle);
    float scrollDelta = io.MouseWheel;

    if (isNormalPanMode) {
        isMiddleMousePressed = isLeftMousePressed;
        isLeftMousePressed = false; 
        isAltPressed = false; 
    }

    editorCamera.ProcessInput(
        io.DeltaTime,
        true,
        isAltPressed,
        isLeftMousePressed,
        isMiddleMousePressed,
        mouseDelta.x,
        -mouseDelta.y,  // Invert Y for standard camera behavior
        scrollDelta
    );
}

void ScenePanel::HandleEntitySelection() {
    // Hover check is now handled by the caller

    // Skip entity selection in normal pan mode
    if (isNormalPanMode) {
        return;  // No entity selection in pan mode
    }

    // Only handle selection on left click (not during camera operations)
    ImGuiIO& io = ImGui::GetIO();
    bool isLeftClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    bool isAltPressed = io.KeyAlt;

    // Only select entities when left clicking without Alt (Alt is for camera orbit)
    if (isLeftClicked && !isAltPressed) {
        // Get mouse position relative to the scene window
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
        ImVec2 contentMax = ImGui::GetWindowContentRegionMax();

        // Calculate relative mouse position within the scene view
        float relativeX = mousePos.x - (windowPos.x + contentMin.x);
        float relativeY = mousePos.y - (windowPos.y + contentMin.y);

        // Get scene view dimensions
        float sceneWidth = contentMax.x - contentMin.x;
        float sceneHeight = contentMax.y - contentMin.y;

        // Check if click is within scene bounds
        if (relativeX >= 0 && relativeX <= sceneWidth &&
            relativeY >= 0 && relativeY <= sceneHeight) {

            // Perform proper raycasting for entity selection
            EditorState& editorState = EditorState::GetInstance();

            // Get camera matrices
            float aspectRatio = sceneWidth / sceneHeight;
            glm::mat4 glmViewMatrix = editorCamera.GetViewMatrix();
            glm::mat4 glmProjMatrix = editorCamera.GetProjectionMatrix(aspectRatio);

            // Convert GLM matrices to Matrix4x4 for raycast
            Matrix4x4 viewMatrix(
                glmViewMatrix[0][0], glmViewMatrix[1][0], glmViewMatrix[2][0], glmViewMatrix[3][0],
                glmViewMatrix[0][1], glmViewMatrix[1][1], glmViewMatrix[2][1], glmViewMatrix[3][1],
                glmViewMatrix[0][2], glmViewMatrix[1][2], glmViewMatrix[2][2], glmViewMatrix[3][2],
                glmViewMatrix[0][3], glmViewMatrix[1][3], glmViewMatrix[2][3], glmViewMatrix[3][3]
            );
            Matrix4x4 projMatrix(
                glmProjMatrix[0][0], glmProjMatrix[1][0], glmProjMatrix[2][0], glmProjMatrix[3][0],
                glmProjMatrix[0][1], glmProjMatrix[1][1], glmProjMatrix[2][1], glmProjMatrix[3][1],
                glmProjMatrix[0][2], glmProjMatrix[1][2], glmProjMatrix[2][2], glmProjMatrix[3][2],
                glmProjMatrix[0][3], glmProjMatrix[1][3], glmProjMatrix[2][3], glmProjMatrix[3][3]
            );

            // Cast ray from camera through mouse position
            RaycastUtil::Ray ray = RaycastUtil::ScreenToWorldRay(
                relativeX, relativeY,
                sceneWidth, sceneHeight,
                viewMatrix, projMatrix
            );

            // Perform raycast against scene entities
            RaycastUtil::RaycastHit hit = RaycastUtil::RaycastScene(ray);

            if (hit.hit) {
                // Entity found, select it
                GUIManager::SetSelectedEntity(hit.entity);
                std::cout << "[ScenePanel] Raycast hit entity " << hit.entity
                          << " at distance " << hit.distance << std::endl;
            } else {
                // No entity hit, clear selection
                GUIManager::SetSelectedEntity(static_cast<Entity>(-1));
                std::cout << "[ScenePanel] Raycast missed - cleared selection" << std::endl;
            }

            std::cout << "[ScenePanel] Mouse clicked at (" << relativeX << ", " << relativeY
                      << ") in scene bounds (" << sceneWidth << "x" << sceneHeight << ")" << std::endl;
        }
    }
}

void ScenePanel::RenderGizmoControls() {
    // Toolbar for gizmo operations with visual feedback for active mode
    if (ImGui::Button(isNormalPanMode ? "[Normal/Pan (Q)]" : "Normal/Pan (Q)")) {
        isNormalPanMode = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(!isNormalPanMode && gizmoOperation == ImGuizmo::TRANSLATE ? "[Translate (W)]" : "Translate (W)")) {
        isNormalPanMode = false;
        gizmoOperation = ImGuizmo::TRANSLATE;
    }
    ImGui::SameLine();
    if (ImGui::Button(!isNormalPanMode && gizmoOperation == ImGuizmo::ROTATE ? "[Rotate (E)]" : "Rotate (E)")) {
        isNormalPanMode = false;
        gizmoOperation = ImGuizmo::ROTATE;
    }
    ImGui::SameLine();
    if (ImGui::Button(!isNormalPanMode && gizmoOperation == ImGuizmo::SCALE ? "[Scale (R)]" : "Scale (R)")) {
        isNormalPanMode = false;
        gizmoOperation = ImGuizmo::SCALE;
    }

    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
}

void ScenePanel::OnImGuiRender() {
    // Update input manager state
    EditorInputManager::Update();

    if (ImGui::Begin(name.c_str(), &isOpen)) {

        // Render gizmo controls at the top
        RenderGizmoControls();
        ImGui::Separator();

        // Handle input (but not if ImGuizmo is active)
        HandleKeyboardInput();

        // Store scene area hover state for camera input
        bool isSceneHovered = false;

        // Get the content region size for the scene view
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        int sceneViewWidth = (int)viewportPanelSize.x;
        int sceneViewHeight = (int)viewportPanelSize.y;

        // Ensure minimum size
        if (sceneViewWidth < 100) sceneViewWidth = 100;
        if (sceneViewHeight < 100) sceneViewHeight = 100;

        // Render the scene with our editor camera
        RenderSceneWithEditorCamera(sceneViewWidth, sceneViewHeight);

        // Get window position for ImGuizmo (where the scene image starts)
        ImVec2 imagePos = ImGui::GetCursorScreenPos();

        // Get the texture from SceneRenderer and display it
        unsigned int sceneTexture = SceneRenderer::GetSceneTexture();
        if (sceneTexture != 0) {
            // Use a child window to contain both the image and ImGuizmo
            ImGui::BeginChild("SceneView", ImVec2((float)sceneViewWidth, (float)sceneViewHeight), false,
                              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            // Get child window position and size
            ImVec2 childPos = ImGui::GetCursorScreenPos();
            ImVec2 childSize = ImGui::GetContentRegionAvail();

            // Draw the scene texture as background
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddImage(
                (void*)(intptr_t)sceneTexture,
                childPos, ImVec2(childPos.x + childSize.x, childPos.y + childSize.y),
                ImVec2(0, 1), ImVec2(1, 0)  // Flip Y coordinate for OpenGL
            );

            // Check if the child window is hovered (without invisible button interfering)
            isSceneHovered = ImGui::IsWindowHovered();

            // Now handle ImGuizmo within the child window context
            HandleImGuizmoInChildWindow((float)sceneViewWidth, (float)sceneViewHeight);

            // Render the ViewGizmo in the top right corner
            RenderViewGizmo((float)sceneViewWidth, (float)sceneViewHeight);

            // End the child window here
            ImGui::EndChild();
        }
        else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Scene View - Framebuffer not ready");
            ImGui::Text("Size: %dx%d", sceneViewWidth, sceneViewHeight);
        }

        // Handle input based on ImGuizmo state (now calculated)
        bool canHandleInput = isSceneHovered && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing();

        if (canHandleInput) {
            HandleCameraInput();
            HandleEntitySelection();
        }
    }
    ImGui::End();
}

void ScenePanel::RenderSceneWithEditorCamera(int width, int height) {
    try {
        // Pass our editor camera data to the rendering system
        SceneRenderer::BeginSceneRender(width, height);
        SceneRenderer::RenderSceneForEditor(
            editorCamera.Position,
            editorCamera.Front,
            editorCamera.Up,
            editorCamera.Zoom
        );
        SceneRenderer::EndSceneRender();

        // Now both the visual representation AND ImGuizmo overlay use our editor camera
        // This gives us proper Unity-style editor controls

    } catch (const std::exception& e) {
        std::cerr << "Exception in RenderSceneWithEditorCamera: " << e.what() << std::endl;
    }
}

void ScenePanel::HandleImGuizmoInChildWindow(float sceneWidth, float sceneHeight) {
    // Ensure ImGuizmo is set up properly for this frame
    ImGuizmo::BeginFrame();

    // Push unique ID for this ImGuizmo instance
    ImGui::PushID("SceneGizmo");

    // Make gizmos bigger and more interactive
    ImGuizmo::SetGizmoSizeClipSpace(0.25f);  // Make gizmos bigger (default is 0.1f)

    // Set ImGuizmo to use the current window's draw list
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

    // Get the current child window dimensions directly
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    // Use the full child window area for ImGuizmo
    ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

    // Enable ImGuizmo to receive input
    ImGuizmo::Enable(true);
    ImGuizmo::AllowAxisFlip(false);


    // Get matrices from editor camera
    float aspectRatio = sceneWidth / sceneHeight;
    glm::mat4 view = editorCamera.GetViewMatrix();
    glm::mat4 projection = editorCamera.GetProjectionMatrix(aspectRatio);

    float viewMatrix[16], projMatrix[16];
    Mat4ToFloatArray(view, viewMatrix);
    Mat4ToFloatArray(projection, projMatrix);

    // Draw grid
    ImGuizmo::DrawGrid(viewMatrix, projMatrix, identityMatrix, 10.0f);

    // Only show gizmo when an entity is selected AND not in normal pan mode
    Entity selectedEntity = GUIManager::GetSelectedEntity();
    if (selectedEntity != static_cast<Entity>(-1) && !isNormalPanMode) {
        // Get the actual transform matrix from the selected entity
        static float selectedObjectMatrix[16];

        // Get transform using RaycastUtil helper to avoid OpenGL header conflicts
        if (!RaycastUtil::GetEntityTransform(selectedEntity, selectedObjectMatrix)) {
            // Fallback to identity if entity doesn't have transform
            memcpy(selectedObjectMatrix, identityMatrix, sizeof(selectedObjectMatrix));
        }

        bool isUsing = ImGuizmo::Manipulate(
            viewMatrix, projMatrix,
            gizmoOperation, gizmoMode,
            selectedObjectMatrix,
            nullptr, nullptr
        );


        // Apply transform changes to the actual entity
        if (isUsing) {
            // Update the entity's transform in the ECS system
            RaycastUtil::SetEntityTransform(selectedEntity, selectedObjectMatrix);

        }
    }

    // Pop the ID scope
    ImGui::PopID();
}

void ScenePanel::RenderViewGizmo(float sceneWidth, float sceneHeight) {
    // Get matrices from editor camera
    float aspectRatio = sceneWidth / sceneHeight;
    glm::mat4 view = editorCamera.GetViewMatrix();
    glm::mat4 projection = editorCamera.GetProjectionMatrix(aspectRatio);

    float viewMatrix[16], projMatrix[16];
    Mat4ToFloatArray(view, viewMatrix);
    Mat4ToFloatArray(projection, projMatrix);

    // Position the ViewGizmo in the top right corner
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    float gizmoSize = 100.0f; // Size of the ViewGizmo
    float margin = 10.0f;     // Margin from the edges

    // Calculate position for top right corner
    float gizmoX = windowPos.x + windowSize.x - gizmoSize - margin;
    float gizmoY = windowPos.y + margin;

    // Set the ViewGizmo position and size
    ImGuizmo::SetRect(gizmoX, gizmoY, gizmoSize, gizmoSize);

    // Create a copy of the view matrix for manipulation
    float manipulatedViewMatrix[16];
    memcpy(manipulatedViewMatrix, viewMatrix, sizeof(manipulatedViewMatrix));

    // Render the ViewGizmo and check if it was manipulated
    ImGuizmo::ViewManipulate(manipulatedViewMatrix, 8.0f,
                             ImVec2(gizmoX, gizmoY),
                             ImVec2(gizmoSize, gizmoSize),
                             0x10101010);

    // Check if the view matrix was modified by comparing with original
    bool wasManipulated = false;
    for (int i = 0; i < 16; ++i) {
        if (std::abs(manipulatedViewMatrix[i] - viewMatrix[i]) > 1e-6f) {
            wasManipulated = true;
            break;
        }
    }

    if (wasManipulated) {
        // Convert the manipulated view matrix back to camera parameters
        glm::mat4 newViewMatrix;
        for (int i = 0; i < 16; ++i) {
            glm::value_ptr(newViewMatrix)[i] = manipulatedViewMatrix[i];
        }

        // Extract camera position and orientation from the inverse view matrix
        glm::mat4 inverseView = glm::inverse(newViewMatrix);
        glm::vec3 newPosition = glm::vec3(inverseView[3]);
        glm::vec3 newFront = -glm::normalize(glm::vec3(inverseView[2]));
        glm::vec3 newUp = glm::normalize(glm::vec3(inverseView[1]));

        // Update the editor camera
        editorCamera.Position = newPosition;
        editorCamera.Front = newFront;
        editorCamera.Up = newUp;
    }
}