#include "RaycastUtil.hpp"
#include <algorithm>
#include <limits>
#include <iostream>
#include <optional>
#include <glm/gtc/type_ptr.hpp>

// Include ECS system from Engine (using configured include paths)
#include "ECS/ECSRegistry.hpp"
#include "Graphics/Model/ModelRenderComponent.hpp"

RaycastUtil::Ray RaycastUtil::ScreenToWorldRay(float mouseX, float mouseY,
                                              float screenWidth, float screenHeight,
                                              const glm::mat4& viewMatrix, const glm::mat4& projMatrix) {
    // Normalize screen coordinates to NDC [-1, 1]
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;  // Flip Y axis

    // Create points in NDC space (near and far plane)
    glm::vec4 rayStartNDC(x, y, -1.0f, 1.0f);  // Near plane
    glm::vec4 rayEndNDC(x, y, 1.0f, 1.0f);     // Far plane

    // Transform to world space
    glm::mat4 invView = glm::inverse(viewMatrix);
    glm::mat4 invProj = glm::inverse(projMatrix);
    glm::mat4 invViewProj = invView * invProj;

    glm::vec4 rayStartWorld = invViewProj * rayStartNDC;
    glm::vec4 rayEndWorld = invViewProj * rayEndNDC;

    // Perform perspective division
    rayStartWorld /= rayStartWorld.w;
    rayEndWorld /= rayEndWorld.w;

    // Create ray
    glm::vec3 rayOrigin(rayStartWorld);
    glm::vec3 rayDirection = glm::normalize(glm::vec3(rayEndWorld - rayStartWorld));

    return Ray(rayOrigin, rayDirection);
}

bool RaycastUtil::RayAABBIntersection(const Ray& ray, const AABB& aabb, float& distance) {
    glm::vec3 invDir = 1.0f / ray.direction;
    glm::vec3 t1 = (aabb.min - ray.origin) * invDir;
    glm::vec3 t2 = (aabb.max - ray.origin) * invDir;

    glm::vec3 tMin = glm::min(t1, t2);
    glm::vec3 tMax = glm::max(t1, t2);

    float tNear = std::max({tMin.x, tMin.y, tMin.z});
    float tFar = std::min({tMax.x, tMax.y, tMax.z});

    // Ray misses the box if tNear > tFar or tFar < 0
    if (tNear > tFar || tFar < 0.0f) {
        return false;
    }

    // Use tNear if it's positive, otherwise use tFar
    distance = (tNear >= 0.0f) ? tNear : tFar;
    return true;
}

RaycastUtil::AABB RaycastUtil::CreateAABBFromTransform(const glm::mat4& transform,
                                                     const glm::vec3& modelSize) {
    // Extract transform components
    glm::vec3 translation(transform[3]);

    // Extract scale from transform matrix
    glm::vec3 scale;
    scale.x = glm::length(glm::vec3(transform[0]));
    scale.y = glm::length(glm::vec3(transform[1]));
    scale.z = glm::length(glm::vec3(transform[2]));

    // Create AABB around the transformed model
    glm::vec3 halfSize = (modelSize * scale) * 0.5f;

    return AABB(translation - halfSize, translation + halfSize);
}

RaycastUtil::RaycastHit RaycastUtil::RaycastScene(const Ray& ray) {
    RaycastHit closestHit;

    try {
        // Get the active ECS manager
        ECSRegistry& registry = ECSRegistry::GetInstance();
        ECSManager& ecsManager = registry.GetActiveECSManager();

        std::cout << "[RaycastUtil] Ray origin: (" << ray.origin.x << ", " << ray.origin.y << ", " << ray.origin.z
                  << ") direction: (" << ray.direction.x << ", " << ray.direction.y << ", " << ray.direction.z << ")" << std::endl;

        // Get all entities with ModelRenderComponent
        // Note: This is a simplified approach. In a real implementation,
        // you'd want to iterate through entities more efficiently

        // For now, we'll test against some known entities
        // In a real implementation, you'd have a system to get all renderable entities

        // Debug: Count total entities with ModelRenderComponent
        int entitiesWithComponent = 0;

        // Test against entities 0-10, but only if they have the component
        for (Entity entity = 0; entity <= 10; ++entity) {
            // Check if entity has ModelRenderComponent before trying to get it
            if (!ecsManager.HasComponent<ModelRenderComponent>(entity)) {
                continue;  // Skip silently - most entities won't have this component
            }

            entitiesWithComponent++;
            std::cout << "[RaycastUtil] Found entity " << entity << " with ModelRenderComponent" << std::endl;

            try {
                // Get the ModelRenderComponent for this entity
                auto& renderComponent = ecsManager.GetComponent<ModelRenderComponent>(entity);

                // Create AABB from the entity's transform
                AABB entityAABB = CreateAABBFromTransform(renderComponent.transform);

                std::cout << "[RaycastUtil] Entity " << entity << " AABB: min("
                          << entityAABB.min.x << ", " << entityAABB.min.y << ", " << entityAABB.min.z
                          << ") max(" << entityAABB.max.x << ", " << entityAABB.max.y << ", " << entityAABB.max.z << ")" << std::endl;

                // Test ray intersection
                float distance;
                if (RayAABBIntersection(ray, entityAABB, distance)) {
                    // Check if this is the closest hit
                    if (!closestHit.hit || distance < closestHit.distance) {
                        closestHit.hit = true;
                        closestHit.entity = entity;
                        closestHit.distance = distance;
                        closestHit.point = ray.origin + ray.direction * distance;
                    }
                }
            } catch (const std::exception& e) {
                // Still catch any other exceptions just in case
                std::cerr << "[RaycastUtil] Error getting component for entity " << entity << ": " << e.what() << std::endl;
                continue;
            }
        }

        std::cout << "[RaycastUtil] Tested " << entitiesWithComponent << " entities with ModelRenderComponent" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[RaycastUtil] Error during raycast: " << e.what() << std::endl;
    }

    return closestHit;
}

bool RaycastUtil::GetEntityTransform(Entity entity, float outMatrix[16]) {
    try {
        // Get the active ECS manager
        ECSRegistry& registry = ECSRegistry::GetInstance();
        ECSManager& ecsManager = registry.GetActiveECSManager();

        if (ecsManager.HasComponent<ModelRenderComponent>(entity)) {
            auto& renderComponent = ecsManager.GetComponent<ModelRenderComponent>(entity);

            // Convert glm::mat4 to float array
            const float* matrixPtr = glm::value_ptr(renderComponent.transform);
            for (int i = 0; i < 16; ++i) {
                outMatrix[i] = matrixPtr[i];
            }

            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "[RaycastUtil] Error getting transform for entity " << entity << ": " << e.what() << std::endl;
    }

    return false;
}

bool RaycastUtil::SetEntityTransform(Entity entity, const float matrix[16]) {
    try {
        // Get the active ECS manager
        ECSRegistry& registry = ECSRegistry::GetInstance();
        ECSManager& ecsManager = registry.GetActiveECSManager();

        if (ecsManager.HasComponent<ModelRenderComponent>(entity)) {
            auto& renderComponent = ecsManager.GetComponent<ModelRenderComponent>(entity);

            // Convert float array back to glm::mat4
            glm::mat4 newTransform;
            for (int i = 0; i < 16; ++i) {
                glm::value_ptr(newTransform)[i] = matrix[i];
            }

            // Update the entity's transform
            renderComponent.transform = newTransform;

            std::cout << "[RaycastUtil] Updated transform for entity " << entity << std::endl;
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "[RaycastUtil] Error setting transform for entity " << entity << ": " << e.what() << std::endl;
    }

    return false;
}