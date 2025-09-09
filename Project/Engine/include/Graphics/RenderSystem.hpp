#pragma once
#include <memory>
#include <vector>
#include "ECS/System.hpp"
#include "Graphics/Model.h"
#include "Graphics/Camera.h"
#include "Graphics/ShaderClass.h"

struct RenderItem {
	std::shared_ptr<Model> Model;
	glm::mat4 transform;
	std::shared_ptr<Shader> Shader;
};

class RenderSystem : public System {
public:
	RenderSystem() = default;
	~RenderSystem() = default;

	bool Initialise(int window_width, int window_height);
	void applyLighting(Shader& shader);
	void Shutdown();

	//void BeginFrame();
	void EndFrame();
	void Clear(float r = 0.2f, float g = 0.3f, float b = 0.3f, float a = 1.0f);

	//void Submit(std::shared_ptr<Model> model, const glm::mat4& transform, std::shared_ptr<Shader> shader);
	void SetCamera(Camera* camera);
	void Render();

	// Remove later
	const glm::vec3* getPointLightPositions() const { return pointLightPositions; }
private:

	//std::vector<RenderItem> renderQueue;
	Camera* currentCamera = nullptr;
	int screenWidth{};
	int screenHeight{};

	// Remove later
	glm::vec3 pointLightPositions[4] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};
};