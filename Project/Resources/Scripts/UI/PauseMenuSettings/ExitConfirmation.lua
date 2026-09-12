local pendingAction = "main_menu"
local M = {}

function M.Open(action)
    pendingAction = action
    local pause = GetComponent(Engine.GetEntityByName("PauseMenuUI"), "ActiveComponent")
    local prompt = GetComponent(Engine.GetEntityByName("ConfirmationPromptUI"), "ActiveComponent")
    local text = GetComponent(Engine.GetEntityByName("ConfirmationText"), "TextRenderComponent")
    if text then
        text.text = action == "quit" and "Quit and lose unsaved progress?"
            or "Return to menu and lose unsaved progress?"
    end
    if pause then pause.isActive = false end
    if prompt then prompt.isActive = true end
end

function M.IsQuit()
    return pendingAction == "quit"
end

return M
