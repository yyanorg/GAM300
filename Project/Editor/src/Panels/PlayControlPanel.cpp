#include "Panels/PlayControlPanel.hpp"
#include "EditorState.hpp"
#include "GUIManager.hpp"
#include "imgui.h"

PlayControlPanel::PlayControlPanel()
    : EditorPanel("Play Controls", true) {
}

void PlayControlPanel::OnImGuiRender() {
    // Get viewport and set fixed position/size for toolbar
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float menuBarHeight = ImGui::GetFrameHeight();

    // Position it at the top center, below the menu bar
    ImVec2 toolbarPos = ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight);
    ImVec2 toolbarSize = ImVec2(viewport->Size.x, ImGui::GetFrameHeight() + 16.0f); // Thicker toolbar

    ImGui::SetNextWindowPos(toolbarPos);
    ImGui::SetNextWindowSize(toolbarSize);

    // Make it non-movable, non-resizable, no title bar
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking;

    // Set window padding for better spacing
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 6.0f));

    if (ImGui::Begin("##PlayControlsToolbar", nullptr, flags)) {
        EditorState& editorState = EditorState::GetInstance();

        // Center the buttons within the toolbar
        ImVec2 windowSize = ImGui::GetContentRegionAvail();

        // Calculate actual button group width more accurately
        float playButtonWidth = 80.0f;
        float stopButtonWidth = 70.0f;
        float spacing = ImGui::GetStyle().ItemSpacing.x * 2; // Space between buttons
        float stateTextWidth = 60.0f; // Estimated width for " | EDIT"
        float totalButtonWidth = playButtonWidth + stopButtonWidth + stateTextWidth + spacing;

        float centerX = (windowSize.x - totalButtonWidth) * 0.5f;
        if (centerX > 0) {
            ImGui::SetCursorPosX(centerX);
        }

        // Add some vertical centering too
        float centerY = (ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight()) * 0.5f;
        if (centerY > 0) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + centerY);
        }

        // Make buttons with good padding
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 4.0f));

        // Play/Pause button
        if (editorState.IsEditMode() || editorState.IsPaused()) {
            if (ImGui::Button("▶ Play", ImVec2(80.0f, 0.0f))) {
                editorState.Play();
                // Auto-focus the Game panel when play is pressed
                auto gamePanel = GUIManager::GetPanelManager().GetPanel("Game");
                if (gamePanel) {
                    gamePanel->SetOpen(true);
                    ImGui::SetWindowFocus("Game");
                }
            }
        } else {
            if (ImGui::Button("⏸ Pause", ImVec2(80.0f, 0.0f))) {
                editorState.Pause();
            }
        }

        ImGui::SameLine();

        // Stop button
        if (ImGui::Button("⏹ Stop", ImVec2(70.0f, 0.0f))) {
            editorState.Stop();
            // Auto-switch to Scene panel when stopping
            auto scenePanel = GUIManager::GetPanelManager().GetPanel("Scene");
            if (scenePanel) {
                scenePanel->SetOpen(true);
                ImGui::SetWindowFocus("Scene");
            }
        }

        ImGui::SameLine();

        // State indicator
        const char* stateText = editorState.IsEditMode() ? "EDIT" :
                               editorState.IsPlayMode() ? "PLAY" :
                               "PAUSED";
        ImGui::TextColored(editorState.IsPlayMode() ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f) :
                          editorState.IsPaused() ? ImVec4(1.0f, 0.6f, 0.0f, 1.0f) :
                          ImVec4(0.7f, 0.7f, 0.7f, 1.0f), " | %s", stateText);

        ImGui::PopStyleVar(); // FramePadding
    }
    ImGui::End();
    ImGui::PopStyleVar(); // WindowPadding
}