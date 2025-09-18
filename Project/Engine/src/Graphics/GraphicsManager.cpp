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

void GraphicsManager::SubmitModel(std::shared_ptr<Model> model, std::shared_ptr<Shader> shader, const Matrix4x4& transform)
{
	if (model && shader) 
	{
		glm::mat4 glmTransform = ConvertMatrix4x4ToGLM(transform);
		auto renderItem = std::make_unique<ModelRenderComponent>(model, shader);
		renderItem->transform = glmTransform;
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
		const TextRenderComponent* textItem = dynamic_cast<const TextRenderComponent*>(renderItem.get());

		if (modelItem) 
		{
			RenderModel(*modelItem);
		}
		else if (textItem) 
		{
			RenderText(*textItem);
		}
		else if (const DebugDrawComponent* debugItem = dynamic_cast<const DebugDrawComponent*>(renderItem.get()))
		{
			RenderDebugDraw(*debugItem);
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

		// Get window dimensions with safety checks
		int windowWidth = WindowManager::GetWindowWidth();
		int windowHeight = WindowManager::GetWindowHeight();

		// Prevent division by zero and ensure minimum dimensions
		if (windowWidth <= 0) windowWidth = 1;
		if (windowHeight <= 0) windowHeight = 1;

		float aspectRatio = (float)windowWidth / (float)windowHeight;

		// Clamp aspect ratio to reasonable bounds to prevent assertion errors
		if (aspectRatio < 0.001f) aspectRatio = 0.001f;
		if (aspectRatio > 1000.0f) aspectRatio = 1000.0f;

		glm::mat4 projection = glm::perspective(
			glm::radians(currentCamera->Zoom),
			aspectRatio,
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

void GraphicsManager::RenderDebugDraw(const DebugDrawComponent& item)
{
	if (!item.isVisible || !item.shader || item.drawCommands.empty()) {
		return;
	}

	// Enable wireframe mode for debug rendering
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Disable depth testing for debug draws so they're always visible
	glDisable(GL_DEPTH_TEST);

	// Activate shader
	item.shader->Activate();

	// Render each draw command
	for (const auto& drawCommand : item.drawCommands) 
	{
		VAO* currentVAO = nullptr;
		unsigned int indexCount = 0;

		// Select appropriate geometry
		switch (drawCommand.type) {
		case DebugDrawType::CUBE:
			currentVAO = item.cubeVAO;
			indexCount = item.cubeIndexCount;
			break;
		case DebugDrawType::SPHERE:
			currentVAO = item.sphereVAO;
			indexCount = item.sphereIndexCount;
			break;
		case DebugDrawType::LINE:
			currentVAO = item.lineVAO;
			indexCount = 2;
			break;
		default:
			continue;
		}

		if (!currentVAO) continue;

		// Create transform matrix
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, drawCommand.position);
		transform = glm::rotate(transform, glm::radians(drawCommand.rotation.x), glm::vec3(1, 0, 0));
		transform = glm::rotate(transform, glm::radians(drawCommand.rotation.y), glm::vec3(0, 1, 0));
		transform = glm::rotate(transform, glm::radians(drawCommand.rotation.z), glm::vec3(0, 0, 1));
		transform = glm::scale(transform, drawCommand.scale);

		// Set up matrices and uniforms
		SetupMatrices(*item.shader, transform);
		item.shader->setVec3("debugColor", drawCommand.color);

		// Bind VAO and render
		currentVAO->Bind();

		if (drawCommand.type == DebugDrawType::LINE) 
		{
			glLineWidth(drawCommand.lineWidth);
			glDrawArrays(GL_LINES, 0, indexCount);
		}
		else 
		{
			glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_INT, 0);
		}

		currentVAO->Unbind();
	}

	// Restore render state
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

glm::mat4 GraphicsManager::ConvertMatrix4x4ToGLM(const Matrix4x4& m)
{
	Matrix4x4 transposed = m.Transposed();
	glm::mat4 converted(
		transposed[0][0], transposed[0][1], transposed[0][2], transposed[0][3],
		transposed[1][0], transposed[1][1], transposed[1][2], transposed[1][3],
		transposed[2][0], transposed[2][1], transposed[2][2], transposed[2][3],
		transposed[3][0], transposed[3][1], transposed[3][2], transposed[3][3]);
	return converted;
}

Matrix4x4 GraphicsManager::ConvertGLMToMatrix4x4(const glm::mat4& m)
{
	// GLM is column-major, Matrix4x4 is row-major, so we need to transpose
	Matrix4x4 converted(
		m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]);
	return converted;
}

