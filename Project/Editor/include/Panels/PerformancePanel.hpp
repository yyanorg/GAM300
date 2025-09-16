#pragma once
#include "EditorPanel.hpp"

/**
 * @brief Panel that displays performance metrics like FPS and delta time
 */
class PerformancePanel : public EditorPanel {
public:
    PerformancePanel();
    virtual ~PerformancePanel() = default;

protected:
    void OnImGuiRender() override;
};