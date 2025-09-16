#include "pch.h"
#include "Graphics/GraphicsManager.hpp"
#include "WindowManager.hpp"

GraphicsManager& GraphicsManager::GetInstance()
{
	static GraphicsManager instance;
	return instance;
}

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

void GraphicsManager::Submit(std::unique_ptr<IRenderComponent> renderItem)
{
	if (renderItem && renderItem->isVisible)
	{
		renderQueue.push_back(std::move(renderItem));
	}
}

void GraphicsManager::SubmitModel(std::shared_ptr<Model> model, std::shared_ptr<Shader> shader, const glm::mat4& transform)
{
	if (model && shader) 
	{
		auto renderItem = std::make_unique<ModelRenderComponent>(model, shader, transform);
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
		[](const std::unique_ptr<IRenderComponent>& a, const std::unique_ptr<IRenderComponent>& b) {
			return a->renderOrder < b->renderOrder;
		});

	// Render all items in the queue
	for (const auto& renderItem : renderQueue) 
	{
		// Cast to ModelRenderItem since that's what we have for now
		// Later you can add a switch statement for different types
		const ModelRenderComponent* modelItem = dynamic_cast<const ModelRenderComponent*>(renderItem.get());
		if (modelItem) 
		{
			RenderModel(*modelItem);
		}
	}
}

void GraphicsManager::RenderModel(const ModelRenderComponent& item)
{
	if (!item.isVisible || !item.model || !item.shader) 
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

void GraphicsManager::SubmitText(const std::string& text, std::shared_ptr<Font> font, std::shared_ptr<Shader> shader, const glm::vec3& position, const glm::vec3& color, float scale, bool is3D, const glm::mat4& transform)
{
	if (font && shader && !text.empty()) 
	{
		auto textItem = std::make_unique<TextRenderComponent>(text, font, shader);
		textItem->position = position;
		textItem->color = color;
		textItem->scale = scale;
		textItem->is3D = is3D;
		textItem->transform = transform;
		Submit(std::move(textItem));
	}
}

void GraphicsManager::RenderText(const TextRenderComponent& item)
{
	if (!item.isVisible || !item.font || !item.shader || item.text.empty()) 
	{
		return;
	}

	// Enable blending for text transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate shader and set uniforms
	item.shader->Activate();
	item.shader->setVec3("textColor", item.color);

	// Set up matrices based on whether it's 2D or 3D text
	if (item.is3D) 
	{
		// 3D text rendering - use normal 3D matrices
		SetupMatrices(*item.shader, item.transform);
	}
	else 
	{
		// 2D screen space text rendering
		Setup2DTextMatrices(*item.shader, item.position, item.scale);
	}

	// Bind VAO and render each character
	glActiveTexture(GL_TEXTURE0);
	VAO* fontVAO = item.font->GetVAO();
	VBO* fontVBO = item.font->GetVBO();

	if (!fontVAO || !fontVBO) 
	{
		std::cerr << "[GraphicsManager] Font VAO/VBO not initialized!" << std::endl;
		glDisable(GL_BLEND);
		return;
	}

	fontVAO->Bind();

	float x = 0.0f;
	float y = 0.0f;

	// Calculate starting position based on alignment
	if (item.alignment == TextRenderComponent::Alignment::CENTER) 
	{
		x = -item.font->GetTextWidth(item.text, item.scale) / 2.0f;
	}

	else if (item.alignment == TextRenderComponent::Alignment::RIGHT) 
	{
		x = -item.font->GetTextWidth(item.text, item.scale);
	}

	// Iterate through all characters
	for (char c : item.text) 
	{
		const Character& ch = item.font->GetCharacter(c);
		if (ch.textureID == 0) {
			std::cerr << "Character '" << c << "' has no texture!" << std::endl;
			continue;
		}

		float xpos = x + ch.bearing.x * item.scale;
		float ypos = y - (ch.size.y - ch.bearing.y) * item.scale;

		float w = ch.size.x * item.scale;
		float h = ch.size.y * item.scale;

		// Update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};

		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		// Update content of VBO memory using your extended VBO class
		fontVBO->UpdateData(vertices, sizeof(vertices));

		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.advance >> 6) * item.scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}

	fontVAO->Unbind();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}

void GraphicsManager::Setup2DTextMatrices(Shader& shader, const glm::vec3& position, float scale)
{
	glm::mat4 projection = glm::ortho(0.0f, (float)WindowManager::GetWindowWidth(), 0.0f, (float)WindowManager::GetWindowHeight());

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);  // Use position as-is
	model = glm::scale(model, glm::vec3(scale, scale, 1.0f));

	shader.setMat4("projection", projection);
	shader.setMat4("model", model);
}

