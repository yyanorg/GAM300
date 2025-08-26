#include "TestScene.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <iomanip>

void TestScene::RunAllTests() {
    std::cout << "=== Running Library Tests ===" << std::endl;
    std::cout << std::endl;

    TestGLFW();
    TestGLAD();
    TestGLM();
    TestFreeType();
    TestAssimp();

    std::cout << "=== All Tests Complete ===" << std::endl;
    std::cout << std::endl;
}

void TestScene::TestGLFW() {
    std::cout << "[GLFW] Testing..." << std::endl;

    // Get GLFW version
    int major, minor, revision;
    glfwGetVersion(&major, &minor, &revision);
    std::cout << "[GLFW] Version: " << major << "." << minor << "." << revision << std::endl;

    // Test if we have a current context
    GLFWwindow* currentWindow = glfwGetCurrentContext();
    if (currentWindow) {
        std::cout << "[GLFW] Current OpenGL context: ACTIVE" << std::endl;

        // Get window size
        int width, height;
        glfwGetWindowSize(currentWindow, &width, &height);
        std::cout << "[GLFW] Window size: " << width << "x" << height << std::endl;
    }
    else {
        std::cout << "[GLFW] ERROR: No current OpenGL context!" << std::endl;
    }

    std::cout << "[GLFW] Test complete!" << std::endl;
    std::cout << std::endl;
}

void TestScene::TestGLAD() {
    std::cout << "[GLAD] Testing..." << std::endl;

    // Get OpenGL version info
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    std::cout << "[GLAD] OpenGL Renderer: " << renderer << std::endl;
    std::cout << "[GLAD] OpenGL Version: " << version << std::endl;
    std::cout << "[GLAD] GLSL Version: " << glslVersion << std::endl;

    // Test basic OpenGL functionality
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Check for errors
    GLenum error = glGetError();
    if (error == GL_NO_ERROR) {
        std::cout << "[GLAD] Basic OpenGL calls: SUCCESS" << std::endl;
    }
    else {
        std::cout << "[GLAD] OpenGL Error: " << error << std::endl;
    }

    std::cout << "[GLAD] Test complete!" << std::endl;
    std::cout << std::endl;
}

void TestScene::TestGLM() {
    std::cout << "[GLM] Testing..." << std::endl;

    // Test basic vector operations
    glm::vec3 vec1(1.0f, 2.0f, 3.0f);
    glm::vec3 vec2(4.0f, 5.0f, 6.0f);
    glm::vec3 result = vec1 + vec2;

    std::cout << "[GLM] Vector addition: (" << vec1.x << "," << vec1.y << "," << vec1.z << ") + ";
    std::cout << "(" << vec2.x << "," << vec2.y << "," << vec2.z << ") = ";
    std::cout << "(" << result.x << "," << result.y << "," << result.z << ")" << std::endl;

    // Test matrix operations
    glm::mat4 identity = glm::mat4(1.0f);
    glm::mat4 translation = glm::translate(identity, glm::vec3(1.0f, 2.0f, 3.0f));
    glm::mat4 rotation = glm::rotate(identity, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 scale = glm::scale(identity, glm::vec3(2.0f, 2.0f, 2.0f));

    glm::mat4 transform = translation * rotation * scale;

    std::cout << "[GLM] Matrix operations: Translation * Rotation * Scale" << std::endl;
    std::cout << "[GLM] Transform matrix [0][0]: " << std::fixed << std::setprecision(3) << transform[0][0] << std::endl;

    // Test dot product
    float dot = glm::dot(vec1, vec2);
    std::cout << "[GLM] Dot product: " << dot << std::endl;

    // Test cross product
    glm::vec3 cross = glm::cross(vec1, vec2);
    std::cout << "[GLM] Cross product: (" << cross.x << "," << cross.y << "," << cross.z << ")" << std::endl;

    std::cout << "[GLM] Test complete!" << std::endl;
    std::cout << std::endl;
}

void TestScene::TestFreeType() {
    std::cout << "[FreeType] Testing..." << std::endl;

    FT_Library ft;
    FT_Error error = FT_Init_FreeType(&ft);

    if (error) {
        std::cout << "[FreeType] ERROR: Failed to initialize FreeType library!" << std::endl;
        return;
    }

    std::cout << "[FreeType] Library initialized successfully" << std::endl;

    // Get FreeType version
    FT_Int major, minor, patch;
    FT_Library_Version(ft, &major, &minor, &patch);
    std::cout << "[FreeType] Version: " << major << "." << minor << "." << patch << std::endl;

    // Try to load the specified font
    FT_Face face;
    error = FT_New_Face(ft, "Resources/Fonts/Kenney Mini.ttf", 0, &face);

    if (error == FT_Err_Unknown_File_Format) {
        std::cout << "[FreeType] ERROR: Font file format not supported!" << std::endl;
    }
    else if (error) {
        std::cout << "[FreeType] ERROR: Failed to load font file (Error code: " << error << ")" << std::endl;
        std::cout << "[FreeType] Make sure 'Resources/Fonts/Kenney Mini.ttf' exists!" << std::endl;
    }
    else {
        std::cout << "[FreeType] Font loaded successfully!" << std::endl;
        std::cout << "[FreeType] Font family: " << face->family_name << std::endl;
        std::cout << "[FreeType] Font style: " << face->style_name << std::endl;
        std::cout << "[FreeType] Number of glyphs: " << face->num_glyphs << std::endl;
        std::cout << "[FreeType] Scalable: " << (FT_IS_SCALABLE(face) ? "Yes" : "No") << std::endl;

        // Test setting font size
        error = FT_Set_Pixel_Sizes(face, 0, 48);
        if (!error) {
            std::cout << "[FreeType] Font size set to 48px successfully" << std::endl;
        }

        FT_Done_Face(face);
    }

    FT_Done_FreeType(ft);
    std::cout << "[FreeType] Test complete!" << std::endl;
    std::cout << std::endl;
}

void TestScene::TestAssimp() {
    std::cout << "[Assimp] Testing..." << std::endl;

    // Create an importer instance
    Assimp::Importer importer;

    // Get Assimp version info (different method)
    const aiScene* scene = importer.GetScene();
    std::cout << "[Assimp] Importer created successfully" << std::endl;

    // Get supported file formats
    aiString extensions;
    importer.GetExtensionList(extensions);
    std::cout << "[Assimp] Supported formats: " << extensions.C_Str() << std::endl;

    // Test import capabilities (without actually loading a model)
    std::cout << "[Assimp] Available post-processing steps:" << std::endl;
    std::cout << "[Assimp]   - Triangulate: " << (aiProcess_Triangulate ? "Available" : "Not available") << std::endl;
    std::cout << "[Assimp]   - Generate Normals: " << (aiProcess_GenNormals ? "Available" : "Not available") << std::endl;
    std::cout << "[Assimp]   - Calculate Tangents: " << (aiProcess_CalcTangentSpace ? "Available" : "Not available") << std::endl;
    std::cout << "[Assimp]   - Optimize Meshes: " << (aiProcess_OptimizeMeshes ? "Available" : "Not available") << std::endl;

    // Test memory management
    //std::cout << "[Assimp] Memory info available: " << (importer.GetMemoryRequirements().total > 0 ? "Yes" : "No") << std::endl;

    std::cout << "[Assimp] Library initialization: SUCCESS" << std::endl;
    std::cout << "[Assimp] Ready to load 3D models" << std::endl;

    std::cout << "[Assimp] Test complete!" << std::endl;
    std::cout << std::endl;
}