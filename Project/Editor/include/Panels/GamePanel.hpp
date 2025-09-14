#pragma once

#include "EditorPanel.hpp"

/**
 * @brief Panel for displaying the game/runtime view.
 * 
 * This panel shows the game as it would appear to the end user,
 * without editor gizmos or tools. It's essentially a preview window.
 */
class GamePanel : public EditorPanel {
public:
    GamePanel();
    virtual ~GamePanel() = default;

    /**
     * @brief Render the game panel's ImGui content.
     */
    void OnImGuiRender() override;

private:
    // No additional members needed for basic functionality
};