#pragma once
#include <memory>
#include "Model.h"

struct ModelComponent {
	std::shared_ptr<Model> model; // contains mesh and textures
	std::shared_ptr<Shader> shader; // shader to use for rendering
	glm::mat4 transform; // model transformation matrix
	bool isVisible;
};