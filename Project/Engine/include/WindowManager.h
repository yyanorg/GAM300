#pragma once

#include <GLFW/glfw3.h>

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class ENGINE_API WindowManager {
public:
    static bool Initialize(int width, int height, const char* title);
    static void Shutdown();
    static GLFWwindow* GetWindow();
    static bool ShouldClose();
    static void SetShouldClose();
    static void PollEvents();
    static void SwapBuffers();

    static int GetWindowWidth();
    static int GetWindowHeight();
    static void SetWindowTitle(const char* title);

    static bool IsFocused();

private:
    static GLFWwindow* window;
    static int width;
    static int height;
    static bool focused;

    static void ErrorCallback(int error, const char* description);
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void WindowFocusCallback(GLFWwindow* window, int focused);
};