#pragma once

class TestScene {
public:
    static bool Initialize();
    static void Update();
    static void Render();
    static void Shutdown();
    static bool IsRunning();
};
