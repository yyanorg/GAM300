#include "Panels/EditorPanel.hpp"
#include "pch.h"

EditorPanel::EditorPanel(const std::string& panelName, bool isOpenByDefault)
    : m_Name(panelName), m_IsOpen(isOpenByDefault) {
    assert(!panelName.empty() && "Panel name cannot be empty");
}