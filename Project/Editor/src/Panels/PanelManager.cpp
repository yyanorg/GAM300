#include "Panels/PanelManager.hpp"
#include <algorithm>

void PanelManager::RegisterPanel(std::shared_ptr<EditorPanel> panel) {
    if (panel && !HasPanel(panel->GetName())) {
        m_Panels.push_back(panel);
        m_PanelMap[panel->GetName()] = panel;
    }
}

void PanelManager::UnregisterPanel(const std::string& panelName) {
    auto it = m_PanelMap.find(panelName);
    if (it != m_PanelMap.end()) {
        // Remove from vector
        m_Panels.erase(
            std::remove_if(m_Panels.begin(), m_Panels.end(),
                [&panelName](const std::shared_ptr<EditorPanel>& panel) {
                    return panel->GetName() == panelName;
                }),
            m_Panels.end());
        
        // Remove from map
        m_PanelMap.erase(it);
    }
}

std::shared_ptr<EditorPanel> PanelManager::GetPanel(const std::string& panelName) {
    auto it = m_PanelMap.find(panelName);
    return (it != m_PanelMap.end()) ? it->second : nullptr;
}

void PanelManager::RenderOpenPanels() {
    for (auto& panel : m_Panels) {
        if (panel && panel->IsOpen()) {
            panel->OnImGuiRender();
        }
    }
}

void PanelManager::TogglePanel(const std::string& panelName) {
    auto panel = GetPanel(panelName);
    if (panel) {
        panel->ToggleOpen();
    }
}

bool PanelManager::HasPanel(const std::string& panelName) const {
    return m_PanelMap.find(panelName) != m_PanelMap.end();
}