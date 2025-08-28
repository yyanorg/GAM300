#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif


class GraphicsManager {
public:
    static void Initialize();
    static void Shutdown();

    // FBO Management - NO EDITOR KNOWLEDGE
    ENGINE_API static bool CreateFBO(int width, int height);
    ENGINE_API static void BindFBO();
    ENGINE_API static void UnbindFBO();
    //static void ResizeFBO(int width, int height);
    ENGINE_API static GLuint GetFBOTexture();
    ENGINE_API static void DeleteFBO();

    // Basic rendering
    static void Clear(float r, float g, float b, float a);
    static void SetViewport(int width, int height);

    // 3D Test content
    static void CreateTestCube();
    static void DrawTestCube();
    static void DeleteTestCube();
    static GLuint fbo;
    static GLuint fboTexture;
    static GLuint depthStencil;
private:
    static int fboWidth, fboHeight;

    static GLuint cubeVAO, cubeVBO;
    static GLuint shaderProgram;
    static float rotation;

    static void CreateShaders();
};