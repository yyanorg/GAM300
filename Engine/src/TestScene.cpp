#include "TestScene.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <vector>

    // ---- 3D Cube ----
    static unsigned int cubeVAO = 0, cubeVBO = 0;
    static unsigned int cubeShaderProgram = 0;

    // ---- Camera ----
    static glm::vec3 cameraPos = { 0.0f, 0.0f, 5.0f };
    static glm::vec3 cameraFront = { 0.0f, 0.0f, -1.0f };
    static glm::vec3 cameraUp = { 0.0f, 1.0f, 0.0f };
    static float cameraSpeed = 2.5f;

    // ---- Window ----
    static GLFWwindow* window = nullptr;
    static bool running = true;

    // ---- FreeType ----
    struct Character {
        unsigned int TextureID;
        glm::ivec2 Size;
        glm::ivec2 Bearing;
        unsigned int Advance;
    };
    static std::map<char, Character> Characters;
    static unsigned int textVAO = 0, textVBO = 0;
    static unsigned int textShaderProgram = 0;

    // ---- Utility Functions ----
    static unsigned int CompileShader(const char* source, GLenum type) {
        unsigned int shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        int success; char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        }
        return shader;
    }

    static unsigned int CreateShaderProgram(const char* vertexSrc, const char* fragmentSrc) {
        unsigned int vertex = CompileShader(vertexSrc, GL_VERTEX_SHADER);
        unsigned int fragment = CompileShader(fragmentSrc, GL_FRAGMENT_SHADER);
        unsigned int program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);
        int success; char infoLog[512];
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Program linking failed: " << infoLog << std::endl;
        }
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return program;
    }

    // ---- Shaders ----
    static const char* cubeVertexShader = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;
out vec3 fragColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    fragColor = aColor;
    gl_Position = projection * view * model * vec4(aPos,1.0);
})";

    static const char* cubeFragmentShader = R"(
#version 330 core
in vec3 fragColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(fragColor,1.0);
})";

    static const char* textVertexShader = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <pos.xy, tex.xy>
out vec2 TexCoords;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(vertex.xy,0.0,1.0);
    TexCoords = vertex.zw;
})";

    static const char* textFragmentShader = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D text;
uniform vec3 textColor;
void main() {
    float alpha = texture(text, TexCoords).r;
    FragColor = vec4(textColor, alpha);
})";

    // ---- Text Rendering ----
    static void RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color) {
        glUseProgram(textShaderProgram);
        glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
        glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(textShaderProgram, "textColor"), color.x, color.y, color.z);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(textVAO);

        for (auto c : text) {
            Character ch = Characters[c];
            float xpos = x + ch.Bearing.x * scale;
            float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
            float w = ch.Size.x * scale;
            float h = ch.Size.y * scale;
            float vertices[6][4] = {
                {xpos, ypos + h, 0.0f,1.0f}, {xpos, ypos, 0.0f,0.0f}, {xpos + w,ypos,1.0f,0.0f},
                {xpos, ypos + h, 0.0f,1.0f}, {xpos + w,ypos,1.0f,0.0f}, {xpos + w,ypos + h,1.0f,1.0f}
            };
            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            glBindBuffer(GL_ARRAY_BUFFER, textVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            x += (ch.Advance >> 6) * scale;
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // ---- Initialize ----
    bool TestScene::Initialize() {
        if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return false; }
        window = glfwCreateWindow(800, 600, "Test Scene", nullptr, nullptr);
        if (!window) { std::cerr << "Window creation failed\n"; glfwTerminate(); return false; }
        glfwMakeContextCurrent(window);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cerr << "GLAD failed\n"; return false; }

        glEnable(GL_DEPTH_TEST);

        // --- Cube Setup ---
        float vertices[] = {
            // positions        // colors
           -0.5f,-0.5f,-0.5f,  1.0f,0.0f,0.0f,
            0.5f,-0.5f,-0.5f,  0.0f,1.0f,0.0f,
            0.5f, 0.5f,-0.5f,  0.0f,0.0f,1.0f,
            0.5f, 0.5f,-0.5f,  0.0f,0.0f,1.0f,
           -0.5f, 0.5f,-0.5f,  1.0f,1.0f,0.0f,
           -0.5f,-0.5f,-0.5f,  1.0f,0.0f,0.0f,

           -0.5f,-0.5f,0.5f,   1.0f,0.0f,1.0f,
            0.5f,-0.5f,0.5f,   0.0f,1.0f,1.0f,
            0.5f, 0.5f,0.5f,   1.0f,1.0f,1.0f,
            0.5f, 0.5f,0.5f,   1.0f,1.0f,1.0f,
           -0.5f, 0.5f,0.5f,   0.0f,0.0f,0.0f,
           -0.5f,-0.5f,0.5f,   1.0f,0.0f,1.0f
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        cubeShaderProgram = CreateShaderProgram(cubeVertexShader, cubeFragmentShader);

        // --- FreeType Setup ---
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) { std::cerr << "Could not init FreeType\n"; return false; }
        FT_Face face;
        if (FT_New_Face(ft, "Resources/Fonts/Kenney Mini.ttf", 0, &face)) { std::cerr << "Failed to load font\n"; return false; }
        FT_Set_Pixel_Sizes(face, 0, 48);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for (unsigned char c = 0;c < 128;c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) { std::cerr << "Failed char " << c << "\n"; continue; }
            unsigned int tex;
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            Character ch = {
                tex,
                {face->glyph->bitmap.width, face->glyph->bitmap.rows},
                {face->glyph->bitmap_left, face->glyph->bitmap_top},
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, ch));

        }
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        textShaderProgram = CreateShaderProgram(textVertexShader, textFragmentShader);
        glGenVertexArrays(1, &textVAO);
        glGenBuffers(1, &textVBO);
        glBindVertexArray(textVAO);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        running = true;
        return true;
    }

    // ---- Update ----
    void TestScene::Update() {
        float delta = 0.016f; // ~60FPS

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * cameraFront * delta;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraFront * delta;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * delta;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * delta;

        if (glfwWindowShouldClose(window)) running = false;
    }

    // ---- Render ----
    void TestScene::Render() {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Cube
        glUseProgram(cubeShaderProgram);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(cubeShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(cubeShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(cubeShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Text
        RenderText("Hello FreeType!", 25.0f, 550.0f, 1.0f, { 1.0f,1.0f,0.0f });

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---- Shutdown ----
    void TestScene::Shutdown() {
        for (auto& c : Characters) glDeleteTextures(1, &c.second.TextureID);
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteBuffers(1, &cubeVBO);
        glDeleteVertexArrays(1, &textVAO);
        glDeleteBuffers(1, &textVBO);
        glDeleteProgram(cubeShaderProgram);
        glDeleteProgram(textShaderProgram);
        glfwDestroyWindow(window);
        glfwTerminate();
        running = false;
    }

    bool TestScene::IsRunning() { return running; }