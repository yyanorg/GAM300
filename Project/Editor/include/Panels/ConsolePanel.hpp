#pragma once

#include "EditorPanel.hpp"
#include "pch.h"
/**
 * @brief Console panel for displaying logs, warnings, and errors.
 * 
 * This panel provides a centralized location for viewing system messages,
 * debug output, and error reports, similar to Unity's Console window.
 */
class ConsolePanel : public EditorPanel {
public:
    ConsolePanel();
    virtual ~ConsolePanel() = default;

    /**
     * @brief Render the console panel's ImGui content.
     */
    void OnImGuiRender() override;

    /**
     * @brief Add a log message to the console.
     * @param message The message to add.
     * @param level The severity level (0=Info, 1=Warning, 2=Error).
     */
    void AddLog(const std::string& message, int level = 0);

    /**
     * @brief Clear all console messages.
     */
    void Clear();

private:
    struct LogEntry {
        std::string message;
        int level; // 0=Info, 1=Warning, 2=Error
        float timestamp;
    };

    std::vector<LogEntry> m_LogEntries;
    bool m_ShowInfo = true;
    bool m_ShowWarnings = true;
    bool m_ShowErrors = true;
    bool m_AutoScroll = true;
    int m_MaxLogEntries = 1000;
};