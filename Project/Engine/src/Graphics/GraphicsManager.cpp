#include "pch.h"
#include "Graphics/GraphicsManager.hpp"
#include "Graphics/Model/ModelRenderItem.hpp"
#include "Graphics/LightManager.hpp"
#include "Graphics/Camera.h"
#include "Graphics/ShaderClass.h"
#include "Graphics/Model/Model.h"
#include "WindowManager.hpp"

bool GraphicsManager::Initialize(int window_width, int window_height)
{
	return false;
}

void GraphicsManager::Shutdown()
{
	renderQueue.clear();
	currentCamera = nullptr;
	std::cout << "[GraphicsManager] Shutdown" << std::endl;
}

void GraphicsManager::BeginFrame()
{
	renderQueue.clear();
}

void GraphicsManager::EndFrame()
{

}

void GraphicsManager::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GraphicsManager::SetCamera(Camera* camera)
{
	currentCamera = camera;
}

void GraphicsManager::Submit(std::unique_ptr<IRenderItem> renderItem)
{
	if (renderItem && renderItem->IsVisible())
	{
		renderQueue.push_back(std::move(renderItem));
	}
}

void GraphicsManager::SubmitModel(std::shared_ptr<Model> model, std::shared_ptr<Shader> shader, const glm::mat4& transform)
{
	if (model && shader) 
	{
		auto renderItem = std::make_unique<ModelRenderItem>(model, shader, transform);
		Submit(std::move(renderItem));
	}
}

void GraphicsManager::Render()
{
	if (!currentCamera) 
	{
		std::cerr << "[GraphicsManager] Warning: No camera set for rendering!" << std::endl;
		return;
	}

	// Sort render queue by render order (lower numbers render first)
	std::sort(renderQueue.begin(), renderQueue.end(),
		[](const std::unique_ptr<IRenderItem>& a, const std::unique_ptr<IRenderItem>& b) {
			return a->GetRenderOrder() < b->GetRenderOrder();
		});

	// Render all items in the queue
	for (const auto& renderItem : renderQueue) 
	{
		// Cast to ModelRenderItem since that's what we have for now
		// Later you can add a switch statement for different types
		const ModelRenderItem* modelItem = dynamic_cast<const ModelRenderItem*>(renderItem.get());
		if (modelItem) 
		{
			RenderModel(*modelItem);
		}
	}
}

void GraphicsManager::RenderModel(const ModelRenderItem& item)
{
	if (!item.IsVisible() || !item.model || !item.shader) 
	{
		return;
	}

	// Activate the shader
	item.shader->Activate();

	// Set up all matrices and uniforms
	SetupMatrices(*item.shader, item.transform);

	// Apply lighting
	ApplyLighting(*item.shader);

	// Draw the model
	item.model->Draw(*item.shader, *currentCamera);
}

void GraphicsManager::ApplyLighting(Shader& shader)
{
	// Moved directly from your ModelSystem::applyLighting method
	LightManager& lightManager = LightManager::getInstance();

	// Apply directional light
	const auto& dirLight = lightManager.getDirectionalLight();
	shader.setVec3("dirLight.direction", dirLight.direction);
	shader.setVec3("dirLight.ambient", dirLight.ambient);
	shader.setVec3("dirLight.diffuse", dirLight.diffuse);
	shader.setVec3("dirLight.specular", dirLight.specular);

	// Apply point lights
	const auto& pointLights = lightManager.getPointLights();
	for (size_t i = 0; i < pointLights.size() && i < 4; i++) 
	{
		std::string base = "pointLights[" + std::to_string(i) + "]";
		shader.setVec3(base + ".position", pointLights[i].position);
		shader.setVec3(base + ".ambient", pointLights[i].ambient);
		shader.setVec3(base + ".diffuse", pointLights[i].diffuse);
		shader.setVec3(base + ".specular", pointLights[i].specular);
		shader.setFloat(base + ".constant", pointLights[i].constant);
		shader.setFloat(base + ".linear", pointLights[i].linear);
		shader.setFloat(base + ".quadratic", pointLights[i].quadratic);
	}

	// Apply spotlight
	if (lightManager.isSpotLightEnabled() && currentCamera) 
	{
		const auto& spotLight = lightManager.getSpotLight();
		shader.setVec3("spotLight.position", currentCamera->Position);
		shader.setVec3("spotLight.direction", currentCamera->Front);
		shader.setVec3("spotLight.ambient", spotLight.ambient);
		shader.setVec3("spotLight.diffuse", spotLight.diffuse);
		shader.setVec3("spotLight.specular", spotLight.specular);
		shader.setFloat("spotLight.constant", spotLight.constant);
		shader.setFloat("spotLight.linear", spotLight.linear);
		shader.setFloat("spotLight.quadratic", spotLight.quadratic);
		shader.setFloat("spotLight.cutOff", spotLight.cutOff);
		shader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);
	}
}

void GraphicsManager::SetupMatrices(Shader& shader, const glm::mat4& modelMatrix)
{
	shader.setMat4("model", modelMatrix);

	if (currentCamera) 
	{
		glm::mat4 view = currentCamera->GetViewMatrix();
		shader.setMat4("view", view);

		glm::mat4 projection = glm::perspective(
			glm::radians(currentCamera->Zoom),
			(float)WindowManager::GetWindowWidth() / (float)WindowManager::GetWindowHeight(),
			0.1f, 100.0f
		);
		shader.setMat4("projection", projection);

		shader.setVec3("cameraPos", currentCamera->Position);
	}
}

