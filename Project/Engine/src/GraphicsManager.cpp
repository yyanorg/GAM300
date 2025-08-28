#include "pch.h"
#include "GraphicsManager.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int GraphicsManager::fboWidth = 800;
int GraphicsManager::fboHeight = 600;

GLuint GraphicsManager::cubeVAO = 0;
GLuint GraphicsManager::cubeVBO = 0;
GLuint GraphicsManager::shaderProgram = 0;
float GraphicsManager::rotation = 0.0f;

GLuint GraphicsManager::fbo = 0;
GLuint GraphicsManager::fboTexture = 0;
GLuint GraphicsManager::depthStencil = 0;

const char* vertexShaderSource = R"(
#version 430 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 fragPos;

void main() {
    fragPos = aPos;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 430 core
in vec3 fragPos;
out vec4 FragColor;

void main() {
    vec3 color = normalize(abs(fragPos));
    FragColor = vec4(color, 1.0);
}
)";

void GraphicsManager::Initialize() {
    std::cout << "[GraphicsManager] Initializing..." << std::endl;

    glEnable(GL_DEPTH_TEST);
    CreateShaders();
    CreateTestCube();
    CreateFBO(800, 600);

    std::cout << "[GraphicsManager] Ready" << std::endl;
}

bool IsOpenGLContextActive()
{
    const GLubyte* version = glGetString(GL_VERSION);
    return version != nullptr;
}


bool GraphicsManager::CreateFBO(int width, int height) {

    if (!IsOpenGLContextActive())
    {
        return false;
    }

    if (fbo != 0)
    {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &fboTexture);
        glDeleteRenderbuffers(1, &depthStencil);
    }

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    glGenRenderbuffers(1, &depthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencil);

    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "FBO initialization failed! Status: " << fboStatus << std::endl;
        return false;
    }

    std::cout << "setup success\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;

    //if (framebuffer != 0) {
    //    DeleteFBO();
    //}

    //fboWidth = width;
    //fboHeight = height;

    //// Create framebuffer
    //glGenFramebuffers(1, &framebuffer);
    //glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    //// Create color texture
    //glGenTextures(1, &colorTexture);
    //glBindTexture(GL_TEXTURE_2D, colorTexture);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    //// Create depth renderbuffer
    //glGenRenderbuffers(1, &depthRenderbuffer);
    //glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

    //if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    //    std::cout << "[GraphicsManager] FBO creation failed!" << std::endl;
    //}

    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //std::cout << "[GraphicsManager] FBO created: " << width << "x" << height << std::endl;
}

void GraphicsManager::BindFBO() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void GraphicsManager::UnbindFBO() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//void GraphicsManager::ResizeFBO(int width, int height) {
//    if (width != fboWidth || height != fboHeight) {
//        CreateFBO(width, height);
//    }
//}

GLuint GraphicsManager::GetFBOTexture() {
    return fboTexture;
}

void GraphicsManager::DeleteFBO() {
    //if (framebuffer) glDeleteFramebuffers(1, &framebuffer);
    //if (colorTexture) glDeleteTextures(1, &colorTexture);
    //if (depthRenderbuffer) glDeleteRenderbuffers(1, &depthRenderbuffer);
    //framebuffer = colorTexture = depthRenderbuffer = 0;
}

void GraphicsManager::Clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GraphicsManager::SetViewport(int width, int height) {
    glViewport(0, 0, width, height);
}

void GraphicsManager::CreateShaders() {
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragment);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void GraphicsManager::CreateTestCube() {
    float vertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,
         // Back face  
         -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,
          0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
          // Left face
          -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
          -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
          // Right face
           0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,
           0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
           // Bottom face
           -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f,
            // Top face
            -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,
             0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void GraphicsManager::DrawTestCube() {

    //BindFBO();

    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);  // Visible blue-gray background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    rotation += 1.0f;

    glUseProgram(shaderProgram);

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.5f, 1.0f, 0.0f));
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)fboWidth / fboHeight, 0.1f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    //UnbindFBO();
}

void GraphicsManager::DeleteTestCube() {
    if (cubeVAO) glDeleteVertexArrays(1, &cubeVAO);
    if (cubeVBO) glDeleteBuffers(1, &cubeVBO);
}

void GraphicsManager::Shutdown() {
    DeleteTestCube();
    DeleteFBO();
    if (shaderProgram) glDeleteProgram(shaderProgram);
    std::cout << "[GraphicsManager] Shutdown complete" << std::endl;
}