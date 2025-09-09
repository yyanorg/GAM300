#pragma once

#include "EditorPanel.hpp"

/**
 * @brief Example panel demonstrating basic ImGui functionality.
 * 
 * This panel replaces the original "Example Window" with a proper panel-based implementation.
 * It serves as a template for creating new editor panels.
 */
class ScenePanel : public EditorPanel {
public:
    ScenePanel();
    virtual ~ScenePanel() = default;

    /**
     * @brief Render the example panel's ImGui content.
     */
    void OnImGuiRender() override;

private:
    float m_SampleFloat = 0.0f;
    int m_Counter = 0;
};