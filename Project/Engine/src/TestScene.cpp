#include "pch.h"
#include "TestScene.hpp"
#include "Input/InputManager.hpp"
#include "ECS/ECSRegistry.hpp"
#include "Asset Manager/AssetManager.hpp"

void TestScene::Initialize() {
	// Initialization code for the test scene
	
	// Initialize GraphicsManager first
	GraphicsManager& gfxManager = GraphicsManager::GetInstance();
	gfxManager.Initialize(WindowManager::GetWindowWidth(), WindowManager::GetWindowHeight());

	// WOON LI TEST CODE
	// Temp testing code - create some ECS managers and entities
	ECSRegistry::GetInstance().CreateECSManager("MainWorld");
	//ECSRegistry::GetInstance().CreateECSManager("SecondaryWorld");
	ECSManager& mainECS = ECSRegistry::GetInstance().GetECSManager("MainWorld");
	//ECSManager& secondaryECS = ECSRegistry::GetInstance().GetECSManager("SecondaryWorld");

	// Create an entity with a Renderer component in the main ECS manager
	Entity testEntt = mainECS.CreateEntity();
	glm::mat4 transform = glm::mat4(1.0f);
	transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, 0.0f));
	transform = glm::scale(transform, glm::vec3(0.1f, 0.1f, 0.1f));
	mainECS.AddComponent<ModelRenderComponent>(testEntt, ModelRenderComponent{ AssetManager::GetInstance().GetAsset<Model>("Resources/Models/backpack/backpack.obj"),
		AssetManager::GetInstance().GetAsset<Shader>("Resources/Shaders/default"),
		transform});

	//mainECS.CreateEntity();
	//mainECS.CreateEntity();
	//mainECS.CreateEntity();
	//mainECS.CreateEntity();

	//mainECS.DestroyEntity(2);

	//secondaryECS.CreateEntity();
	//secondaryECS.CreateEntity();
	//secondaryECS.CreateEntity();

	//secondaryECS.ClearAllEntities();

	// GRAPHICS TEST CODE
	mainECS.modelSystem->Initialise();

	// Loads model
	//backpackModel = std::make_shared<Model>("Resources/Models/backpack/backpack.obj");
	//shader = std::make_shared<Shader>("Resources/Shaders/default.vert", "Resources/Shaders/default.frag");

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

void TestScene::Update() {
	// Update logic for the test scene
	ECSManager& mainECS = ECSRegistry::GetInstance().GetECSManager("MainWorld");
	GraphicsManager& gfxManager = GraphicsManager::GetInstance();

	float currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	processInput();

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

	gfxManager.Render();

	// 5. Draw light cubes manually (temporary - you can make this a system later)
	DrawLightCubes();

	// 6. End frame
	gfxManager.EndFrame();
	// Update systems.
}

void TestScene::Exit() {
	// Cleanup code for the test scene

	// Exit systems.

	std::cout << "TestScene Exited" << std::endl;
}

void TestScene::processInput()
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

	double xpos = InputManager::GetMouseX();
	double ypos = InputManager::GetMouseY();

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement((float)xoffset, (float)yoffset);
}

void TestScene::DrawLightCubes() 
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