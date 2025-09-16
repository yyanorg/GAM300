#include "pch.h"
#include <Scene/SceneInstance.hpp>
#include <Input/InputManager.hpp>
#include <ECS/ECSRegistry.hpp>
#include <Asset Manager/AssetManager.hpp>
#include <Transform/TransformComponent.hpp>
#include <Graphics/TextRendering/TextUtils.hpp>
#include "ECS/NameComponent.hpp"

void SceneInstance::Initialize() {
	// Initialization code for the scene
	
	// Initialize GraphicsManager first
	GraphicsManager& gfxManager = GraphicsManager::GetInstance();
	gfxManager.Initialize(WindowManager::GetWindowWidth(), WindowManager::GetWindowHeight());

	// WOON LI TEST CODE
	ECSManager& ecsManager = ECSRegistry::GetInstance().GetECSManager(scenePath);

	// Create a backpack entity with a Renderer component in the main ECS manager
	Entity backpackEntt = ecsManager.CreateEntity();
	ecsManager.AddComponent<Transform>(backpackEntt, Transform{});
	Transform& backpacktransform = ecsManager.GetComponent<Transform>(backpackEntt);
	backpacktransform.position = { 0, 0, 0 };
	backpacktransform.scale = { .1f, .1f, .1f };
	backpacktransform.rotation = { 0, 0, 0 };
	ecsManager.AddComponent<NameComponent>(backpackEntt, NameComponent{"dora the explorer"});
	ecsManager.AddComponent<ModelRenderComponent>(backpackEntt, ModelRenderComponent{ AssetManager::GetInstance().GetAsset<Model>("Resources/Models/backpack/backpack.obj"),
		AssetManager::GetInstance().GetAsset<Shader>("Resources/Shaders/default")});

	Entity backpackEntt2 = ecsManager.CreateEntity();
	ecsManager.AddComponent<Transform>(backpackEntt2, Transform{});
	Transform& backpacktransform2 = ecsManager.GetComponent<Transform>(backpackEntt2);
	backpacktransform2.position = { 1, -0.5f, 0 };
	backpacktransform2.scale = { .2f, .2f, .2f };
	backpacktransform2.rotation = { 0, 0, 0 };
	ecsManager.AddComponent<NameComponent>(backpackEntt2, NameComponent{ "ash ketchum" });
	ecsManager.AddComponent<ModelRenderComponent>(backpackEntt2, ModelRenderComponent{ AssetManager::GetInstance().GetAsset<Model>("Resources/Models/backpack/backpack.obj"),
		AssetManager::GetInstance().GetAsset<Shader>("Resources/Shaders/default")});

	// GRAPHICS TEST CODE
	ecsManager.transformSystem->Initialise();
	ecsManager.modelSystem->Initialise();

	// Text
	auto textShader = std::make_shared<Shader>();
	if (textShader->LoadAsset("Resources/Shaders/text")) {
		std::cout << "Text shader loaded successfully!" << std::endl;
	}
	else {
		std::cout << "Failed to load text shader!" << std::endl;
	}
	Entity text = ecsManager.CreateEntity();
	ecsManager.AddComponent<TextRenderComponent>(text, TextRenderComponent{ "Hello World!", AssetManager::GetInstance().GetAsset<Font>("Resources/Fonts/Kenney Mini.ttf"), textShader });
	TextRenderComponent& textComp = ecsManager.GetComponent<TextRenderComponent>(text);
	TextUtils::SetPosition(textComp, glm::vec3(800,0,0));
	TextUtils::SetAlignment(textComp, TextRenderComponent::Alignment::CENTER);

	// Creates light
	lightShader = std::make_shared<Shader>();
	lightShader->LoadAsset("Resources/Shaders/light");
	std::vector<std::shared_ptr<Texture>> emptyTextures = {};
	lightCubeMesh = std::make_shared<Mesh>(lightVertices, lightIndices, emptyTextures);

	// Sets camera
	gfxManager.SetCamera(&camera);

	// Initialize systems.

	std::cout << "TestScene Initialized" << std::endl;
}

void SceneInstance::Update(double dt) {
	dt;

	// Update logic for the test scene
	ECSManager& mainECS = ECSRegistry::GetInstance().GetECSManager(scenePath);

	processInput((float)WindowManager::getDeltaTime());

	// Update systems.
	mainECS.transformSystem->update();
}

void SceneInstance::Draw() {
	ECSManager& mainECS = ECSRegistry::GetInstance().GetECSManager(scenePath);

	GraphicsManager& gfxManager = GraphicsManager::GetInstance();
	//RenderSystem::getInstance().BeginFrame();
	gfxManager.BeginFrame();
	gfxManager.Clear();

	//glm::mat4 transform = glm::mat4(1.0f);
	//transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, 0.0f));
	//transform = glm::scale(transform, glm::vec3(0.1f, 0.1f, 0.1f));
	//RenderSystem::getInstance().Submit(backpackModel, transform, shader);

	gfxManager.SetCamera(&camera);
	if (mainECS.modelSystem)
	{
		mainECS.modelSystem->Update();
	}
	if (mainECS.textSystem)
	{
		mainECS.textSystem->Update();
	}

	gfxManager.Render();

	// 5. Draw light cubes manually (temporary - you can make this a system later)
	DrawLightCubes();

	// 6. End frame
	gfxManager.EndFrame();
}

void SceneInstance::Exit() {
	// Cleanup code for the test scene

	// Exit systems.
	//ECSRegistry::GetInstance().GetECSManager(scenePath).modelSystem->Exit();

	std::cout << "TestScene Exited" << std::endl;
}

void SceneInstance::processInput(float deltaTime)
{
	if (InputManager::GetKeyDown(GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(WindowManager::getWindow(), true);

	float cameraSpeed = 2.5f * deltaTime;
	if (InputManager::GetKey(GLFW_KEY_W))
		camera.Position += cameraSpeed * camera.Front;
	if (InputManager::GetKey(GLFW_KEY_S))
		camera.Position -= cameraSpeed * camera.Front;
	if (InputManager::GetKey(GLFW_KEY_A))
		camera.Position -= glm::normalize(glm::cross(camera.Front, camera.Up)) * cameraSpeed;
	if (InputManager::GetKey(GLFW_KEY_D))
		camera.Position += glm::normalize(glm::cross(camera.Front, camera.Up)) * cameraSpeed;

	float xpos = (float)InputManager::GetMouseX();
	float ypos = (float)InputManager::GetMouseY();

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void SceneInstance::DrawLightCubes() 
{
	// Get light positions from LightManager instead of renderSystem
	LightManager& lightManager = LightManager::getInstance();
	const auto& pointLights = lightManager.getPointLights();

	// Draw light cubes at point light positions
	for (size_t i = 0; i < pointLights.size() && i < 4; i++) {
		lightShader->Activate();

		// Set up matrices for light cube
		glm::mat4 lightModel = glm::mat4(1.0f);
		lightModel = glm::translate(lightModel, pointLights[i].position);
		lightModel = glm::scale(lightModel, glm::vec3(0.2f)); // Make them smaller

		// Set up view and projection matrices
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(
			glm::radians(camera.Zoom),
			(float)WindowManager::GetWindowWidth() / (float)WindowManager::GetWindowHeight(),
			0.1f, 100.0f
		);

		lightShader->setMat4("model", lightModel);
		lightShader->setMat4("view", view);
		lightShader->setMat4("projection", projection);
		//lightShader->setVec3("lightColor", pointLights[i].diffuse); // Use light color

		lightCubeMesh->Draw(*lightShader, camera);
	}
}

void SceneInstance::DrawLightCubes(const Camera& cameraOverride)
{
	// Get light positions from LightManager instead of renderSystem
	LightManager& lightManager = LightManager::getInstance();
	const auto& pointLights = lightManager.getPointLights();

	// Draw light cubes at point light positions
	for (size_t i = 0; i < pointLights.size() && i < 4; i++) {
		lightShader->Activate();

		// Set up matrices for light cube
		glm::mat4 lightModel = glm::mat4(1.0f);
		lightModel = glm::translate(lightModel, pointLights[i].position);
		lightModel = glm::scale(lightModel, glm::vec3(0.2f)); // Make them smaller

		// Set up view and projection matrices using the override camera
		glm::mat4 view = cameraOverride.GetViewMatrix();
		glm::mat4 projection = glm::perspective(
			glm::radians(cameraOverride.Zoom),
			(float)WindowManager::GetWindowWidth() / (float)WindowManager::GetWindowHeight(),
			0.1f, 100.0f
		);

		lightShader->setMat4("model", lightModel);
		lightShader->setMat4("view", view);
		lightShader->setMat4("projection", projection);
		//lightShader->setVec3("lightColor", pointLights[i].diffuse); // Use light color

		lightCubeMesh->Draw(*lightShader, cameraOverride);
	}
}