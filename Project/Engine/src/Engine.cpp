#include "pch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Graphics/LightManager.hpp"

#include "Engine.h"

#include "WindowManager.hpp"
#include "Input/InputManager.hpp"
#include "Asset Manager/MetaFilesManager.hpp"
#include "ECS/ECSRegistry.hpp"
#include "TestScene.hpp"

namespace TEMP {
	std::string windowTitle = "GAM300";
}

TestScene Engine::testScene;

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

//RenderSystem& renderer = RenderSystem::getInstance();
//std::shared_ptr<Model> backpackModel;
//std::shared_ptr<Shader> shader;
////----------------LIGHT-------------------
//std::shared_ptr<Shader> lightShader;
//std::shared_ptr<Mesh> lightCubeMesh;

bool Engine::Initialize() {

	WindowManager::Initialize(SCR_WIDTH, SCR_HEIGHT, TEMP::windowTitle.c_str());

    std::cout << "[Engine] Initializing..." << std::endl;

	// WOON LI TEST CODE
	InputManager::Initialize(WindowManager::getWindow());
	MetaFilesManager::InitializeAssetMetaFiles("Resources");

	// Test scene
	Engine::testScene.Initialize();

	// ---Set Up Lighting---
	LightManager& lightManager = LightManager::getInstance();
	const auto& pointLights = lightManager.getPointLights();
	// Set up directional light
	lightManager.setDirectionalLight(
		glm::vec3(-0.2f, -1.0f, -0.3f),
		glm::vec3(0.4f, 0.4f, 0.4f)
	);

	// Add point lights
	glm::vec3 lightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	for (int i = 0; i < 4; i++) 
	{
		lightManager.addPointLight(lightPositions[i], glm::vec3(0.8f, 0.8f, 0.8f));
	}

	// Set up spotlight
	lightManager.setSpotLight(
		glm::vec3(0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
	);

	//lightManager.printLightStats();

	return true;
}

void Engine::Update() {
	Engine::testScene.Update();
}

void Engine::StartDraw() {
}

void Engine::Draw() 
{
	
}

void Engine::EndDraw() {
	glfwSwapBuffers(WindowManager::getWindow());
	InputManager::Update();
	glfwPollEvents();
}

void Engine::Shutdown() {
    std::cout << "[Engine] Shutdown complete" << std::endl;
}

bool Engine::IsRunning() {
	return !WindowManager::ShouldClose();
}