#pragma once
#include <memory>
#include <vector>
#include "ECS/System.hpp"
#include "Model.h"
#include "Graphics/Camera.h"
#include "Graphics/ShaderClass.h"

class ModelSystem : public System {
public:
    ModelSystem() = default;
    ~ModelSystem() = default;

    bool Initialise();  // Remove window parameters - GraphicsManager handles this
    void Update();      // Changed from Render() to Update()
    void Shutdown();
};