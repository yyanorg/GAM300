#include "Panels/PlayControlPanel.hpp"
#include "EditorState.hpp"
#include "GUIManager.hpp"
#include "imgui.h"

PlayControlPanel::PlayControlPanel()
    : EditorPanel("Play Controls", true) {
}

void PlayControlPanel::OnImGuiRender() {
    if (ImGui::Begin(m_Name.c_str(), &m_IsOpen, ImGuiWindowFlags_NoScrollbar)) {

        EditorState& editorState = EditorState::GetInstance();

        // Center the buttons within the panel
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        float buttonGroupWidth = 200.0f; // Estimated total width
        float centerX = (windowSize.x - buttonGroupWidth) * 0.5f;
        if (centerX > 0) {
            ImGui::SetCursorPosX(centerX);
        }

        // Make buttons thicker
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 8.0f));

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

        ImGui::PopStyleVar();
    }
    ImGui::End();
}