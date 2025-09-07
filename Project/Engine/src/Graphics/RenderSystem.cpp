#include "pch.h"
#include "Graphics/RenderSystem.hpp"
#include "ECS/ECSRegistry.hpp"
#include <Graphics/Renderer.hpp>


bool RenderSystem::Initialise(int window_width, int window_height)
{
    screenWidth = window_width;
    screenHeight = window_height;

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, screenWidth, screenHeight);
    std::cout << "[Renderer] Initialised" << std::endl;
	return true;
}

void RenderSystem::SetUpLighting(Shader& shader)
{
	// positions of the point lights
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	// directional light
	shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
	shader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
	// point light 1
	shader.setVec3("pointLights[0].position", pointLightPositions[0]);
	shader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	shader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	shader.setFloat("pointLights[0].constant", 1.0f);
	shader.setFloat("pointLights[0].linear", 0.09f);
	shader.setFloat("pointLights[0].quadratic", 0.032f);
	// point light 2
	shader.setVec3("pointLights[1].position", pointLightPositions[1]);
	shader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	shader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
	shader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	shader.setFloat("pointLights[1].constant", 1.0f);
	shader.setFloat("pointLights[1].linear", 0.09f);
	shader.setFloat("pointLights[1].quadratic", 0.032f);
	// point light 3
	shader.setVec3("pointLights[2].position", pointLightPositions[2]);
	shader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	shader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
	shader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
	shader.setFloat("pointLights[2].constant", 1.0f);
	shader.setFloat("pointLights[2].linear", 0.09f);
	shader.setFloat("pointLights[2].quadratic", 0.032f);
	// point light 4
	shader.setVec3("pointLights[3].position", pointLightPositions[3]);
	shader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	shader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
	shader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
	shader.setFloat("pointLights[3].constant", 1.0f);
	shader.setFloat("pointLights[3].linear", 0.09f);
	shader.setFloat("pointLights[3].quadratic", 0.032f);
	// spotLight
	shader.setVec3("spotLight.position", currentCamera->Position);
	shader.setVec3("spotLight.direction", currentCamera->Front);
	shader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	shader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	shader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
	shader.setFloat("spotLight.constant", 1.0f);
	shader.setFloat("spotLight.linear", 0.09f);
	shader.setFloat("spotLight.quadratic", 0.032f);
	shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}

//void RenderSystem::BeginFrame()
//{
//    renderQueue.clear();
//}

void RenderSystem::Clear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//void RenderSystem::Submit(std::shared_ptr<Model> model, const glm::mat4& transform, std::shared_ptr<Shader> shader)
//{
//    renderQueue.push_back({ model, transform, shader });
//}

void RenderSystem::SetCamera(Camera* camera)
{
    currentCamera = camera;
}

void RenderSystem::Render()
{
	ECSManager& ecsManager = ECSRegistry::GetInstance().GetActiveECSManager();
	for (const auto& entity : entities) {
		auto& renderer = ecsManager.GetComponent<Renderer>(entity);
		renderer.shader->Activate();
		renderer.shader->setMat4("model", renderer.transform);

		if (currentCamera)
		{
			// View, for camera
			// Camera explanation:
			// cameraPos      - The position of the camera in world space
			// cameraFront    - The direction the camera is facing (default is (0, 0, -1), into the screen)
			// cameraPos + cameraFront - The target point the camera is looking at
			// 
			// We use glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp) to generate the view matrix:
			// - The first argument is the camera position
			// - The second argument is the position we are looking at (target)
			// - The third argument is the up direction (usually (0, 1, 0))
			// 
			// Adding cameraFront to cameraPos gives us a point directly in front of the camera.
			// Technically the direction will never change, but lookAt asks for a target, hence in order to keep it in line with the direction we add the camera pos
			// If we dont add camera position, we will constantly be looking at that point when we want to move forward instead
			// This allows the camera to move and look in the right direction based on its position and orientation.

			glm::mat4 view = currentCamera->GetViewMatrix();
			glm::mat4 projection = glm::perspective(glm::radians(currentCamera->Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

			renderer.shader->setMat4("view", view);
			renderer.shader->setMat4("projection", projection);
			renderer.shader->setVec3("cameraPos", currentCamera->Position);

			// Material
			renderer.shader->setFloat("material.shininess", 32.0f);

			SetUpLighting(*renderer.shader);
		}

		renderer.model->Draw(*renderer.shader, *currentCamera);
	}

   // for (const auto& item : renderQueue)
   // {
   //     item.Shader->Activate();
   //     item.Shader->setMat4("model", item.transform);
   //     
   //     if (currentCamera)
   //     {
			//// View, for camera
			//// Camera explanation:
			//// cameraPos      - The position of the camera in world space
			//// cameraFront    - The direction the camera is facing (default is (0, 0, -1), into the screen)
			//// cameraPos + cameraFront - The target point the camera is looking at
			//// 
			//// We use glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp) to generate the view matrix:
			//// - The first argument is the camera position
			//// - The second argument is the position we are looking at (target)
			//// - The third argument is the up direction (usually (0, 1, 0))
			//// 
			//// Adding cameraFront to cameraPos gives us a point directly in front of the camera.
			//// Technically the direction will never change, but lookAt asks for a target, hence in order to keep it in line with the direction we add the camera pos
			//// If we dont add camera position, we will constantly be looking at that point when we want to move forward instead
			//// This allows the camera to move and look in the right direction based on its position and orientation.

   //         glm::mat4 view = currentCamera->GetViewMatrix();
   //         glm::mat4 projection = glm::perspective(glm::radians(currentCamera->Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

   //         item.Shader->setMat4("view", view);
   //         item.Shader->setMat4("projection", projection);
   //         item.Shader->setVec3("cameraPos", currentCamera->Position);

			//// Material
			//item.Shader->setFloat("material.shininess", 32.0f);

			//SetUpLighting(*item.Shader);
   //     }

   //     item.Model->Draw(*item.Shader, *currentCamera);
   // }
}

void RenderSystem::EndFrame()
{

}

void RenderSystem::Shutdown()
{
    //renderQueue.clear();
}