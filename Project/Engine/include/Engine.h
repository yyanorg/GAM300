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

    // Rendering phases
    static void StartDraw();
    static void Draw();
    static void EndDraw();

    static void Shutdown();
    static bool IsRunning();

    // FBO Control (no editor knowledge)
    static void SetRenderToFramebuffer(bool enabled);
    static unsigned int GetFramebufferTexture();
    //static void ResizeFramebuffer(int width, int height);
};