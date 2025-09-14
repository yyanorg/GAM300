#pragma once

#include "EditorPanel.hpp"

/**
 * @brief Panel containing play, pause, and stop buttons for game control.
 *
 * This panel provides a resizable interface for controlling game state,
 * separate from the main menu bar.
 */
class PlayControlPanel : public EditorPanel {
public:
    PlayControlPanel();
    virtual ~PlayControlPanel() = default;

    /**
     * @brief Render the play control panel's ImGui content.
     */
    void OnImGuiRender() override;

private:
    // No additional members needed for now
};