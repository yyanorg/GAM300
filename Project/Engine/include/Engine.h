#pragma once

// #ifdef ENGINE_EXPORTS
// #define ENGINE_API __declspec(dllexport)
// #else
// #define ENGINE_API __declspec(dllimport)
// #endif

// Cross-platform API export/import macros
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

struct GLFWwindow;

class ENGINE_API Engine {
public:
    static bool Initialize();
    static void Update();

    // Rendering phases
    static void StartDraw();
    static void Draw();
    static void EndDraw();

    static bool IsRunning();
    static void Shutdown();

    static GLFWwindow* GetWindowTemp(); // remove after windowmanager
};