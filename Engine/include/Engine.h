#pragma once

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class ENGINE_API Engine {
public:
    static void Initialize();
    static void Update();
    static void Shutdown();
    static bool IsRunning();
};
