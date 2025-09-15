#pragma once

#include "Platform/Platform.h"
#include <string>
#include <glm/glm.hpp>

#ifdef _WIN32
#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif
#else
// Linux/GCC
#ifdef ENGINE_EXPORTS
#define ENGINE_API __attribute__((visibility("default")))
#else
#define ENGINE_API
#endif
#endif

class ENGINE_API WindowManager {
public:
    static bool Initialize(GLint width, GLint height, const char* title);

    static void UpdateViewportDimensions();

    static void Exit();

    static PlatformWindow getWindow();

    static void SetWindowShouldClose();

    static bool ShouldClose();
    
    static void SwapBuffers();
    static void PollEvents();

    static void error_cb(int error, char const* description);
    static void fbsize_cb(PlatformWindow ptr_win, int width, int height);

    static GLint GetWindowWidth();
    static GLint GetWindowHeight();

    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    static GLint GetViewportWidth();
    static GLint GetViewportHeight();

    static void SetWindowTitle(const char* title);

    static void ToggleFullscreen();
    static void MinimizeWindow();

    static bool IsWindowMinimized();
    static bool IsWindowFocused();
    static void window_focus_callback(PlatformWindow window, int focused);

    static void updateDeltaTime();
    static double getDeltaTime();
    static double getFps();

    // Scene framebuffer functions
    static unsigned int CreateSceneFramebuffer(int width, int height);
    static void DeleteSceneFramebuffer();
    static unsigned int GetSceneTexture();
    static void BeginSceneRender(int width, int height);
    static void EndSceneRender();
    static void RenderScene();
    static void RenderSceneForEditor();
    static void RenderSceneForEditor(const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp, float cameraZoom);

private:

    static bool isFocused;
    static bool isFullscreen;      // Tracks whether the window is fullscreen
    static GLint windowedWidth;    // Saved width for windowed mode
    static GLint windowedHeight;   // Saved height for windowed mode
    static GLint windowedPosX;     // Saved X position for windowed mode
    static GLint windowedPosY;     // Saved Y position for windowed mode

    static class IPlatform* platform;
    static PlatformWindow ptrWindow;

    static GLint width;
    static GLint height;

    static GLint viewportWidth;
    static GLint viewportHeight;

    static const char* title;

    static double deltaTime;
    static double lastFrameTime;
};
