#pragma once
#include "../include/Graphics/IRenderItem.hpp"
#include <memory>
#include <glm/glm.hpp>

class Model;
class Shader;
class Camera;

class ModelRenderItem : public IRenderItem {
public:
	std::shared_ptr<Model> model;
	std::shared_ptr<Shader> shader;
	glm::mat4 transform;
	bool isVisible = true;

	ModelRenderItem(std::shared_ptr<Model> m, std::shared_ptr<Shader> s, const glm::mat4& t) 
		: model(std::move(m)), shader(std::move(s)), transform(std::move(t)){}

	int GetRenderOrder() const override { return 100; }
	bool IsVisible() const override { return isVisible && model && shader; }
};
