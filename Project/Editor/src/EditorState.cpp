#include "EditorState.hpp"
#include <iostream>

EditorState& EditorState::GetInstance() {
    static EditorState instance;
    return instance;
}

void EditorState::SetState(State newState) {
    if (m_CurrentState != newState) {
        State oldState = m_CurrentState;
        m_CurrentState = newState;
        
        // Log state changes for debugging
        const char* stateNames[] = { "EDIT_MODE", "PLAY_MODE", "PAUSED" };
        std::cout << "[EditorState] State changed from " << stateNames[static_cast<int>(oldState)]
                  << " to " << stateNames[static_cast<int>(newState)] << std::endl;
    }
}

void EditorState::Play() {
    if (m_CurrentState == State::EDIT_MODE) {
        SetState(State::PLAY_MODE);
    } else if (m_CurrentState == State::PAUSED) {
        SetState(State::PLAY_MODE);
    }
}

void EditorState::Pause() {
    if (m_CurrentState == State::PLAY_MODE) {
        SetState(State::PAUSED);
    }
}

void EditorState::Stop() {
    SetState(State::EDIT_MODE);
}

void EditorState::SetSelectedEntity(Entity entity) {
    if (m_SelectedEntity != entity) {
        m_SelectedEntity = entity;
        std::cout << "[EditorState] Selected entity: " << entity << std::endl;
    }
}

void EditorState::ClearSelection() {
    if (m_SelectedEntity != INVALID_ENTITY) {
        std::cout << "[EditorState] Cleared selection" << std::endl;
        m_SelectedEntity = INVALID_ENTITY;
    }
}