#pragma once

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class ENGINE_API Engine {
public:
    static bool Initialize();
    static void Update();

    // Rendering phases
    static void StartDraw();
    static void Draw();
    static void EndDraw();

    static void Shutdown();
};