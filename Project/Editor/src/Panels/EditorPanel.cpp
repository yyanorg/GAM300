#include "../include/Panels/EditorPanel.hpp"

EditorPanel::EditorPanel(const std::string& panelName, bool isOpenByDefault)
    : m_Name(panelName), m_IsOpen(isOpenByDefault) {
}