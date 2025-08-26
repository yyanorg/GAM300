#pragma once

class EditorManager {
public:
    static void Initialize();
    static void Update();
    static void Render();
    static void Shutdown();

private:
    static bool initialized;
};