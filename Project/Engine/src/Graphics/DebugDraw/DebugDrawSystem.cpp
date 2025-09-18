#include "pch.h"
#include "Graphics/DebugDraw/DebugDrawSystem.hpp"
#include "Graphics/DebugDraw/DebugDrawComponent.hpp"
#include "ECS/ECSRegistry.hpp"
#include "Graphics/GraphicsManager.hpp"
#include "Transform/TransformComponent.hpp"
#include <glm/gtc/matrix_transform.hpp>

bool DebugDrawSystem::Initialise() 
{
    // Create debug shader (you'll need to create this shader file)
    debugShader = std::make_shared<Shader>("shaders/debug.vert", "shaders/debug.frag");

    CreatePrimitiveGeometry();

    std::cout << "[DebugDrawSystem] Initialized" << std::endl;
    return true;
}

void DebugDrawSystem::Update()
{
    ECSManager& ecsManager = ECSRegistry::GetInstance().GetActiveECSManager();
    GraphicsManager& gfxManager = GraphicsManager::GetInstance();

    // Process all entities with DebugDrawComponent
    for (const auto& entity : entities) 
    {
        auto& debugComponent = ecsManager.GetComponent<DebugDrawComponent>(entity);

        if (!debugComponent.isVisible || !debugComponent.HasCommands()) 
        {
            continue;
        }

        // Set up shared geometry references if not already set
        if (!debugComponent.cubeVAO) 
        {
            debugComponent.shader = debugShader;
            debugComponent.cubeVAO = cubeGeometry.vao.get();
            debugComponent.sphereVAO = sphereGeometry.vao.get();
            debugComponent.lineVAO = lineGeometry.vao.get();
            debugComponent.cubeIndexCount = cubeGeometry.indexCount;
            debugComponent.sphereIndexCount = sphereGeometry.indexCount;
        }

        // Submit the debug component directly to graphics manager
        auto debugComponentCopy = std::make_unique<DebugDrawComponent>(debugComponent);
        gfxManager.Submit(std::move(debugComponentCopy));
    }

    // Update timed commands (remove expired ones)
    UpdateTimedCommands(0.016f);
}

void DebugDrawSystem::Shutdown()
{
    std::cout << "[DebugDrawSystem] Shutdown" << std::endl;
}

void DebugDrawSystem::CreatePrimitiveGeometry()
{
    CreateCubeGeometry();
    CreateSphereGeometry();
    CreateLineGeometry();
}

void DebugDrawSystem::CreateCubeGeometry()
{
    std::vector<glm::vec3> vertices = {
        // Bottom face
        {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f,  0.5f}, {-0.5f, -0.5f,  0.5f},
        // Top face  
        {-0.5f,  0.5f, -0.5f}, {0.5f,  0.5f, -0.5f},
        {0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}
    };

    std::vector<GLuint> indices = {
        // Bottom face
        0, 1, 1, 2, 2, 3, 3, 0,
        // Top face
        4, 5, 5, 6, 6, 7, 7, 4,
        // Vertical edges
        0, 4, 1, 5, 2, 6, 3, 7
    };

    cubeGeometry.vao = std::make_unique<VAO>();
    cubeGeometry.vbo = std::make_unique<VBO>(vertices.size() * sizeof(glm::vec3), GL_STATIC_DRAW);
    cubeGeometry.ebo = std::make_unique<EBO>(indices);

    cubeGeometry.vao->Bind();
    cubeGeometry.vbo->UpdateData(vertices.data(), vertices.size() * sizeof(glm::vec3));

    // Position attribute
    cubeGeometry.vao->LinkAttrib(*cubeGeometry.vbo, 0, 3, GL_FLOAT, sizeof(glm::vec3), (void*)0);

    cubeGeometry.indexCount = indices.size();
    cubeGeometry.vao->Unbind();
}

void DebugDrawSystem::CreateSphereGeometry()
{
    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices;

    const int segments = 16;
    const int rings = 8;

    // Generate sphere wireframe
    for (int ring = 0; ring <= rings; ++ring) 
    {
        float phi = glm::pi<float>() * ring / rings;
        for (int segment = 0; segment <= segments; ++segment) 
        {
            float theta = 2.0f * glm::pi<float>() * segment / segments;

            float x = sin(phi) * cos(theta);
            float y = cos(phi);
            float z = sin(phi) * sin(theta);

            vertices.push_back(glm::vec3(x, y, z) * 0.5f);
        }
    }

    // Generate indices for wireframe
    for (int ring = 0; ring < rings; ++ring) 
    {
        for (int segment = 0; segment < segments; ++segment) 
        {
            int current = ring * (segments + 1) + segment;
            int next = current + segments + 1;

            // Horizontal lines
            indices.push_back(current);
            indices.push_back(current + 1);

            // Vertical lines
            if (ring < rings - 1) {
                indices.push_back(current);
                indices.push_back(next);
            }
        }
    }

    sphereGeometry.vao = std::make_unique<VAO>();
    sphereGeometry.vbo = std::make_unique<VBO>(vertices.size() * sizeof(glm::vec3), GL_STATIC_DRAW);
    sphereGeometry.ebo = std::make_unique<EBO>(indices);

    sphereGeometry.vao->Bind();
    sphereGeometry.vbo->UpdateData(vertices.data(), vertices.size() * sizeof(glm::vec3));

    sphereGeometry.vao->LinkAttrib(*sphereGeometry.vbo, 0, 3, GL_FLOAT, sizeof(glm::vec3), (void*)0);

    sphereGeometry.indexCount = indices.size();
    sphereGeometry.vao->Unbind();
}

void DebugDrawSystem::CreateLineGeometry()
{
    std::vector<glm::vec3> vertices = 
    {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f}
    };

    lineGeometry.vao = std::make_unique<VAO>();
    lineGeometry.vbo = std::make_unique<VBO>(vertices.size() * sizeof(glm::vec3), GL_STATIC_DRAW);

    lineGeometry.vao->Bind();
    lineGeometry.vbo->UpdateData(vertices.data(), vertices.size() * sizeof(glm::vec3));
    lineGeometry.vao->LinkAttrib(*lineGeometry.vbo, 0, 3, GL_FLOAT, sizeof(glm::vec3), (void*)0);

    lineGeometry.indexCount = 2;
    lineGeometry.vao->Unbind();
}

void DebugDrawSystem::SubmitDebugRenderItems()
{
}

void DebugDrawSystem::UpdateTimedCommands(float deltaTime)
{
    ECSManager& ecsManager = ECSRegistry::GetInstance().GetActiveECSManager();

    for (const auto& entity : entities) {
        auto& debugComponent = ecsManager.GetComponent<DebugDrawComponent>(entity);

        // Remove expired commands
        debugComponent.drawCommands.erase(
            std::remove_if(debugComponent.drawCommands.begin(), debugComponent.drawCommands.end(),
                [deltaTime](DebugDrawData& cmd) {
                    if (cmd.duration > 0.0f) {
                        cmd.duration -= deltaTime;
                        return cmd.duration <= 0.0f;
                    }
                    return false;
                }),
            debugComponent.drawCommands.end()
        );
    }
}

void DebugDrawSystem::DrawCube(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& color, float duration)
{
}

void DebugDrawSystem::DrawSphere(const glm::vec3& position, float radius, const glm::vec3& color, float duration)
{
}

void DebugDrawSystem::DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color, float duration, float width)
{
}

void DebugDrawSystem::DrawMeshWireframe(std::shared_ptr<Model> model, const glm::vec3& position, const glm::vec3& color, float duration)
{
}
