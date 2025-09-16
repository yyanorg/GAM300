#pragma once
#include "../include/Graphics/IRenderComponent.hpp"
#include <memory>
#include <glm/glm.hpp>

class Model;
class Shader;
class Camera;

class ModelRenderComponent : public IRenderComponent {
public:
	std::shared_ptr<Model> model;
	std::shared_ptr<Shader> shader;
	glm::mat4 transform;

	ModelRenderComponent(std::shared_ptr<Model> m, std::shared_ptr<Shader> s) 
		: model(std::move(m)), shader(std::move(s)), transform(){}
	ModelRenderComponent() = default;
	~ModelRenderComponent() = default;

	//int GetRenderOrder() const override { return 100; }
	//bool IsVisible() const override { return isVisible && model && shader; }
};
