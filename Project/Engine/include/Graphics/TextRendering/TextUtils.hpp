#pragma once
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "TextRenderComponent.hpp"

class Font;

class TextUtils {
public:
    // Text content manipulation
    static void SetText(TextRenderComponent& comp, const std::string& newText);

    // Color utilities
    static void SetColor(TextRenderComponent& comp, const glm::vec3& newColor);
    static void SetColor(TextRenderComponent& comp, float r, float g, float b);

    // Position utilities (2D screen space)
    static void SetPosition(TextRenderComponent& comp, const glm::vec3& newPosition);
    static void SetPosition(TextRenderComponent& comp, float x, float y, float z = 0.0f);

    // Scale and alignment
    static void SetScale(TextRenderComponent& comp, float newScale);
    static void SetAlignment(TextRenderComponent& comp, TextRenderComponent::Alignment newAlignment);

    // 3D world space positioning
    static void SetWorldTransform(TextRenderComponent& comp, const glm::mat4& newTransform);
    static void SetWorldPosition(TextRenderComponent& comp, const glm::vec3& worldPos);
    static void SetWorldPosition(TextRenderComponent& comp, float x, float y, float z);

    // Dimension calculations
    static float GetEstimatedWidth(const TextRenderComponent& comp);
    static float GetEstimatedHeight(const TextRenderComponent& comp);

    // Validation
    static bool IsValid(const TextRenderComponent& comp);

    // Advanced utilities
    static void CenterOnScreen(TextRenderComponent& comp, int screenWidth, int screenHeight);
    static void SetScreenAnchor(TextRenderComponent& comp, int screenWidth, int screenHeight,
        float anchorX, float anchorY); // 0.0-1.0 values
    static glm::vec2 GetTextDimensions(const TextRenderComponent& comp);
};