#pragma once

class UIManager {
public:
    static void Initialize();
    static void StartRender();
    static void Render();
    static void EndRender();
    static void Shutdown();

private:
    static bool initialized;
    static void CreateDockSpace();
    static void ShowViewport();
    static void ShowProperties();
    static void ShowConsole();
};