#include "Panels/GamePanel.hpp"
#include "imgui.h"
#include "Graphics/SceneRenderer.hpp"
#include "EditorState.hpp"
#include "Engine.h"
#include <algorithm>
#include <cmath>

GamePanel::GamePanel()
    : EditorPanel("Game", true), selectedResolutionIndex(0), useCustomAspectRatio(false),
      customAspectRatio(16.0f / 9.0f), freeAspect(true) {

    // Initialize common resolutions
    resolutions.emplace_back(1920, 1080, "Full HD (1920x1080)");
    resolutions.emplace_back(1280, 720, "HD (1280x720)");
    resolutions.emplace_back(1600, 900, "HD+ (1600x900)");

    // Android device resolutions (portrait)
    resolutions.emplace_back(1080, 2400, "Galaxy S21 (1080x2400)");
    resolutions.emplace_back(1440, 3200, "Galaxy S22 Ultra (1440x3200)");
    resolutions.emplace_back(1080, 2340, "Pixel 7 (1080x2340)");

    // iPhone device resolutions (portrait)
    resolutions.emplace_back(1179, 2556, "iPhone 14 Pro (1179x2556)");
    resolutions.emplace_back(1284, 2778, "iPhone 14 Pro Max (1284x2778)");
}

void GamePanel::OnImGuiRender() {
    if (ImGui::Begin(m_Name.c_str(), &m_IsOpen)) {

        // Render the resolution panel toolbar
        RenderResolutionPanel();

        // Get available space after toolbar
        ImVec2 availableSize = ImGui::GetContentRegionAvail();
        int availableWidth = (int)availableSize.x;
        int availableHeight = (int)availableSize.y;

        // Ensure minimum size
        if (availableWidth < 100) availableWidth = 100;
        if (availableHeight < 100) availableHeight = 100;

        // Always render at a fixed base resolution (e.g., 1920x1080 for 16:9)
        const int baseRenderWidth = 1920;
        const int baseRenderHeight = 1080;

        // Calculate display viewport dimensions with aspect ratio preservation
        int displayWidth, displayHeight;
        float offsetX, offsetY;
        CalculateViewportDimensions(availableWidth, availableHeight,
                                  displayWidth, displayHeight, offsetX, offsetY);

        EditorState& editorState = EditorState::GetInstance();

        // Set cursor position to center the viewport
        ImVec2 startPos = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(startPos.x + offsetX, startPos.y + offsetY));

        // Always render the scene at base resolution (no stretching)
        SceneRenderer::BeginSceneRender(baseRenderWidth, baseRenderHeight);

        if (Engine::ShouldRunGameLogic() || Engine::IsPaused()) {
            // Render 3D scene with game logic running
            SceneRenderer::RenderScene(); // This will run with game logic
        } else {
            // Render scene preview with game camera (no game logic)
            SceneRenderer::RenderScene(); // This should show the game camera view
        }

        SceneRenderer::EndSceneRender();

        // Get the texture from SceneRenderer and display it
        unsigned int sceneTexture = SceneRenderer::GetSceneTexture();
        if (sceneTexture != 0) {
            // Calculate crop UV coordinates based on target aspect ratio
            float targetAspectRatio;
            if (freeAspect) {
                targetAspectRatio = (float)displayWidth / (float)displayHeight;
            } else if (useCustomAspectRatio) {
                targetAspectRatio = customAspectRatio;
            } else {
                const auto& res = resolutions[selectedResolutionIndex];
                targetAspectRatio = (float)res.width / (float)res.height;
            }

            float baseAspectRatio = (float)baseRenderWidth / (float)baseRenderHeight;

            // Calculate UV coordinates for cropping
            ImVec2 uv0, uv1;
            if (targetAspectRatio > baseAspectRatio) {
                // Target is wider - crop top/bottom
                float cropHeight = baseAspectRatio / targetAspectRatio;
                float cropOffset = (1.0f - cropHeight) * 0.5f;
                uv0 = ImVec2(0, 1.0f - cropOffset);        // Bottom-left
                uv1 = ImVec2(1, cropOffset);               // Top-right
            } else {
                // Target is taller - crop left/right
                float cropWidth = targetAspectRatio / baseAspectRatio;
                float cropOffset = (1.0f - cropWidth) * 0.5f;
                uv0 = ImVec2(cropOffset, 1);               // Bottom-left
                uv1 = ImVec2(1.0f - cropOffset, 0);        // Top-right
            }

            ImGui::Image(
                (void*)(intptr_t)sceneTexture,
                ImVec2((float)displayWidth, (float)displayHeight),
                uv0, uv1  // Use calculated crop coordinates
            );

            // Draw border around viewport for clarity
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetItemRectMin();
            ImVec2 pos_max = ImGui::GetItemRectMax();
            draw_list->AddRect(pos, pos_max, IM_COL32(100, 100, 100, 255));
        }
        else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Game View - Framebuffer not ready");
            ImGui::Text("Size: %dx%d", displayWidth, displayHeight);
        }
    }
    ImGui::End();
}

void GamePanel::RenderResolutionPanel() {
    // Begin toolbar
    if (ImGui::BeginChild("ResolutionToolbar", ImVec2(0, 35), true, ImGuiWindowFlags_NoScrollbar)) {

        // Resolution dropdown
        ImGui::Text("Resolution:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200);

        std::string previewText;
        if (freeAspect) {
            previewText = "Free Aspect";
        } else if (useCustomAspectRatio) {
            previewText = "Custom Aspect (" + std::to_string(customAspectRatio) + ":1)";
        } else {
            previewText = resolutions[selectedResolutionIndex].name;
        }

        if (ImGui::BeginCombo("##Resolution", previewText.c_str())) {
            // Free aspect option
            if (ImGui::Selectable("Free Aspect", freeAspect)) {
                freeAspect = true;
                useCustomAspectRatio = false;
            }

            // Preset resolutions
            for (size_t i = 0; i < resolutions.size(); i++) {
                bool isSelected = !freeAspect && !useCustomAspectRatio && (i == selectedResolutionIndex);
                if (ImGui::Selectable(resolutions[i].name.c_str(), isSelected)) {
                    selectedResolutionIndex = (int)i;
                    freeAspect = false;
                    useCustomAspectRatio = false;
                }
            }

            // Custom aspect ratio option
            if (ImGui::Selectable("Custom Aspect", useCustomAspectRatio)) {
                useCustomAspectRatio = true;
                freeAspect = false;
            }

            ImGui::EndCombo();
        }

        // Custom aspect ratio input
        if (useCustomAspectRatio) {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(80);
            ImGui::DragFloat("##AspectRatio", &customAspectRatio, 0.01f, 0.1f, 10.0f, "%.2f");
        }

        // Info display
        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        if (freeAspect) {
            ImGui::Text("Mode: Free Aspect");
        } else if (useCustomAspectRatio) {
            ImGui::Text("Aspect: %.2f:1", customAspectRatio);
        } else {
            const auto& res = resolutions[selectedResolutionIndex];
            float aspectRatio = (float)res.width / (float)res.height;
            ImGui::Text("%dx%d (%.2f:1)", res.width, res.height, aspectRatio);
        }
    }
    ImGui::EndChild();
}

void GamePanel::CalculateViewportDimensions(int availableWidth, int availableHeight,
                                          int& viewportWidth, int& viewportHeight,
                                          float& offsetX, float& offsetY) {
    if (freeAspect) {
        // Use full available space
        viewportWidth = availableWidth;
        viewportHeight = availableHeight;
        offsetX = 0.0f;
        offsetY = 0.0f;
        return;
    }

    // Calculate target aspect ratio
    float targetAspectRatio;
    if (useCustomAspectRatio) {
        targetAspectRatio = customAspectRatio;
    } else {
        const auto& res = resolutions[selectedResolutionIndex];
        targetAspectRatio = (float)res.width / (float)res.height;
    }

    // Calculate viewport dimensions maintaining aspect ratio
    float availableAspectRatio = (float)availableWidth / (float)availableHeight;

    if (availableAspectRatio > targetAspectRatio) {
        // Available area is wider than target - letterbox horizontally
        viewportHeight = availableHeight;
        viewportWidth = (int)(availableHeight * targetAspectRatio);
        offsetX = (availableWidth - viewportWidth) * 0.5f;
        offsetY = 0.0f;
    } else {
        // Available area is taller than target - letterbox vertically
        viewportWidth = availableWidth;
        viewportHeight = (int)(availableWidth / targetAspectRatio);
        offsetX = 0.0f;
        offsetY = (availableHeight - viewportHeight) * 0.5f;
    }

    // Ensure minimum dimensions
    if (viewportWidth < 100) viewportWidth = 100;
    if (viewportHeight < 100) viewportHeight = 100;
}