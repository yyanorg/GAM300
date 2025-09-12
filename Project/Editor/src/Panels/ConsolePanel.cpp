#include "Panels/ConsolePanel.hpp"
#include "imgui.h"
#include <algorithm>

// Include Engine logging to access the GUI queue
#include "Logging.hpp"

ConsolePanel::ConsolePanel() 
    : EditorPanel("Console", true) {
    // Add some initial log messages
    AddLog("Editor initialized successfully", 0);
    AddLog("Loading assets...", 0);
    AddLog("Some non-critical warning", 1);
}

void ConsolePanel::OnImGuiRender() {
    // IMPORTANT: Drain Engine log queue FIRST, before any ImGui rendering
    // This ensures new messages appear immediately
    DrainEngineLogQueue();

    if (ImGui::Begin(m_Name.c_str(), &m_IsOpen)) {
        // Filter checkboxes
        ImGui::Checkbox("Info", &m_ShowInfo); ImGui::SameLine();
        ImGui::Checkbox("Warnings", &m_ShowWarnings); ImGui::SameLine();
        ImGui::Checkbox("Errors", &m_ShowErrors); ImGui::SameLine();
        ImGui::Checkbox("Auto Scroll", &m_AutoScroll);

        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            Clear();
        }

        ImGui::Separator();

        // Console output area
        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

        // Display log entries
        for (const auto& entry : m_LogEntries) {
            bool show = false;
            ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default white

            switch (entry.level) {
                case 0: // Info/Debug/Trace
                    show = m_ShowInfo;
                    color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Light gray
                    break;
                case 1: // Warning
                    show = m_ShowWarnings;
                    color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
                    break;
                case 2: // Error/Critical
                    show = m_ShowErrors;
                    color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); // Red
                    break;
            }

            if (show) {
                ImGui::PushStyleColor(ImGuiCol_Text, color);
                
                // Add timestamp and level prefix
                const char* levelStr = (entry.level == 0) ? "[INFO]" : 
                                      (entry.level == 1) ? "[WARN]" : "[ERROR]";
                ImGui::Text("[%.2f] %s %s", entry.timestamp, levelStr, entry.message.c_str());
                
                ImGui::PopStyleColor();
            }
        }

        // Auto-scroll to bottom
        if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();

        // Command input area
        ImGui::Separator();
        static char inputBuffer[256] = "";
        ImGui::PushItemWidth(-80);
        if (ImGui::InputText("##ConsoleInput", inputBuffer, sizeof(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (strlen(inputBuffer) > 0) {
                AddLog(std::string("Command: ") + inputBuffer, 0);
                // TODO: Process command
                memset(inputBuffer, 0, sizeof(inputBuffer));
            }
        }
        ImGui::PopItemWidth();

        ImGui::SameLine();
        if (ImGui::Button("Send")) {
            if (strlen(inputBuffer) > 0) {
                AddLog(std::string("Command: ") + inputBuffer, 0);
                // TODO: Process command
                memset(inputBuffer, 0, sizeof(inputBuffer));
            }
        }
    }
    ImGui::End();
}

void ConsolePanel::DrainEngineLogQueue() {
    // Get the GUI log queue from the Engine
    EngineLogging::GuiLogQueue& logQueue = EngineLogging::GetGuiLogQueue();

    EngineLogging::LogMessage message("", EngineLogging::LogLevel::Info);

    // Drain all pending messages from the queue
    while (logQueue.TryPop(message)) {
        // Convert Engine log level to our console panel level
        int level = ConvertEngineLogLevel(message.level);

        // Use the timestamp provided by the engine message
        // Create log entry
        LogEntry entry;
        entry.message = message.text;
        entry.level = level;
        entry.timestamp = message.timestamp;

        m_LogEntries.push_back(entry);

        // Keep only the most recent entries
        if (m_LogEntries.size() > static_cast<size_t>(m_MaxLogEntries)) {
            m_LogEntries.erase(m_LogEntries.begin());
        }
    }
}

int ConsolePanel::ConvertEngineLogLevel(EngineLogging::LogLevel level) {
    switch (level) {
        case EngineLogging::LogLevel::Trace:
        case EngineLogging::LogLevel::Debug:
        case EngineLogging::LogLevel::Info:
            return 0; // Info
        case EngineLogging::LogLevel::Warn:
            return 1; // Warning
        case EngineLogging::LogLevel::Error:
        case EngineLogging::LogLevel::Critical:
            return 2; // Error
        default:
            return 0; // Default to Info
    }
}

void ConsolePanel::AddLog(const std::string& message, int level) {
    LogEntry entry;
    entry.message = message;
    entry.level = level;
    entry.timestamp = ImGui::GetTime();

    m_LogEntries.push_back(entry);

    // Keep only the most recent entries
    if (m_LogEntries.size() > static_cast<size_t>(m_MaxLogEntries)) {
        m_LogEntries.erase(m_LogEntries.begin());
    }
}

void ConsolePanel::Clear() {
    m_LogEntries.clear();
}