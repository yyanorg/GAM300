#pragma once

#include "Engine.h"

class GameManager {
public:
    static void Initialize();
    static void Update();
    static void Shutdown();

private:
    static bool initialized;
};