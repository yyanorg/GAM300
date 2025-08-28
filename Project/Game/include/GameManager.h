#pragma once

class GameManager {
public:
    // Static methods for global access
    static void Initialize();
    static void Update();
    static void Shutdown();

private:
    // Static member to track initialization
    static bool initialized;
};
