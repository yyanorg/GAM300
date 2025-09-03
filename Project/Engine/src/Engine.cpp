#include "pch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Graphics/Model.h"

#include "Engine.h"

#include "WindowManager.hpp"
#include "Input/InputManager.hpp"
#include "ECS/ECSRegistry.hpp"

namespace TEMP {
	std::string windowTitle = "GAM300";
}

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

Shader* shaderProgram;
Model* backPack;
Shader* lightShader;
Mesh* lightCubeMesh;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// positions of the point lights
glm::vec3 pointLightPositions[] = {
	glm::vec3(0.7f,  0.2f,  2.0f),
	glm::vec3(2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3(0.0f,  0.0f, -3.0f)
};

bool Engine::Initialize() {

	WindowManager::Initialize(SCR_WIDTH, SCR_HEIGHT, TEMP::windowTitle.c_str());

    std::cout << "[Engine] Initializing..." << std::endl;

	glm::vec3 cubePositions[]{
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	std::vector<Vertex> vertices = {
		// Back face (4 vertices: 0-3)
		{{-0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 0
		{{ 0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 1
		{{ 0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 2
		{{-0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 3

		// Front face (4 vertices: 4-7)
		{{-0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 4
		{{ 0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 5
		{{ 0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 6
		{{-0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 7

		// Left face (4 vertices: 8-11)
		{{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 8
		{{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 9
		{{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 10
		{{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 11

		// Right face (4 vertices: 12-15)
		{{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 12
		{{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 13
		{{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 14
		{{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 15

		// Bottom face (4 vertices: 16-19)
		{{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 16
		{{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 17
		{{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 18
		{{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 19

		// Top face (4 vertices: 20-23)
		{{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 20
		{{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 21
		{{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 22
		{{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}  // 23
	};

	std::vector<GLuint> indices = {
		// Back face
		0, 1, 2,   2, 3, 0,
		// Front face
		4, 5, 6,   6, 7, 4,
		// Left face
		8, 9, 10,  10, 11, 8,
		// Right face
		12, 13, 14, 14, 15, 12,
		// Bottom face
		16, 17, 18, 18, 19, 16,
		// Top face
		20, 21, 22, 22, 23, 20
	};

	std::vector<Vertex> lightVertices = {
		// Back face (4 vertices: 0-3)
		{{-0.1f, -0.1f, -0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 0
		{{ 0.1f, -0.1f, -0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 1
		{{ 0.1f,  0.1f, -0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 2
		{{-0.1f,  0.1f, -0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 3

		// Front face (4 vertices: 4-7)
		{{-0.1f, -0.1f,  0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 4
		{{ 0.1f, -0.1f,  0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 5
		{{ 0.1f,  0.1f,  0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 6
		{{-0.1f,  0.1f,  0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 7

		// Left face (4 vertices: 8-11)
		{{-0.1f,  0.1f,  0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 8
		{{-0.1f,  0.1f, -0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 9
		{{-0.1f, -0.1f, -0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 10
		{{-0.1f, -0.1f,  0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 11

		// Right face (4 vertices: 12-15)
		{{ 0.1f,  0.1f,  0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 12
		{{ 0.1f,  0.1f, -0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 13
		{{ 0.1f, -0.1f, -0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 14
		{{ 0.1f, -0.1f,  0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 15

		// Bottom face (4 vertices: 16-19)
		{{-0.1f, -0.1f, -0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 16
		{{ 0.1f, -0.1f, -0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 17
		{{ 0.1f, -0.1f,  0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 18
		{{-0.1f, -0.1f,  0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 19

		// Top face (4 vertices: 20-23)
		{{-0.1f,  0.1f, -0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 20
		{{ 0.1f,  0.1f, -0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 21
		{{ 0.1f,  0.1f,  0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 22
		{{-0.1f,  0.1f,  0.1f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}  // 23
	};

	std::vector<GLuint> lightIndices = {
		// Back face
		0, 1, 2,   2, 3, 0,
		// Front face
		4, 5, 6,   6, 7, 4,
		// Left face
		8, 9, 10,  10, 11, 8,
		// Right face
		12, 13, 14, 14, 15, 12,
		// Bottom face
		16, 17, 18, 18, 19, 16,
		// Top face
		20, 21, 22, 22, 23, 20
	};


	glfwSetCursorPosCallback(WindowManager::getWindow(), mouse_callback);
	glfwSetScrollCallback(WindowManager::getWindow(), scroll_callback);
	glfwSetInputMode(WindowManager::getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	Texture textures[]
	{
		Texture("Resources/Textures/container2.png", "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE),
		Texture("Resources/Textures/container2_specular.png", "specular", 1, GL_RGBA, GL_UNSIGNED_BYTE)
	};

	// Generates Shader object using shaders defualt.vert and default.frag
	shaderProgram = new Shader("Resources/Shaders/default.vert", "Resources/Shaders/default.frag");
	/*std::vector<Texture> textureVector = { textures[0], textures[1] };
	Mesh cubesMesh(vertices, indices, textureVector);*/
	backPack = new Model("Resources/Models/backpack/backpack.obj");
	//Model ourModel("Resources/Models/FinalBaseMesh.obj");
	//----------------LIGHT-------------------
	lightShader = new Shader("Resources/Shaders/light.vert", "Resources/Shaders/light.frag");
	std::vector<Texture> emptyTextures = {}; // No textures needed for light cube
	lightCubeMesh = new Mesh(lightVertices, lightIndices, emptyTextures);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// WOON LI TEST CODE

	// Temp testing code - create some ECS managers and entities
	ECSRegistry::GetInstance().CreateECSManager("MainWorld");
    ECSRegistry::GetInstance().GetActiveECSManager().CreateEntity();
    ECSRegistry::GetInstance().GetActiveECSManager().CreateEntity();
    ECSRegistry::GetInstance().GetActiveECSManager().CreateEntity();
    ECSRegistry::GetInstance().GetActiveECSManager().CreateEntity();
    ECSRegistry::GetInstance().GetActiveECSManager().CreateEntity();

    ECSRegistry::GetInstance().GetActiveECSManager().DestroyEntity(2);

	ECSRegistry::GetInstance().CreateECSManager("SecondaryWorld");
	ECSRegistry::GetInstance().GetECSManager("SecondaryWorld").CreateEntity();
	ECSRegistry::GetInstance().GetECSManager("SecondaryWorld").CreateEntity();
	ECSRegistry::GetInstance().GetECSManager("SecondaryWorld").CreateEntity();

    ECSRegistry::GetInstance().GetActiveECSManager().ClearAllEntities();

    std::cout << "[Engine] successfully initialized!" << std::endl;

	return true;
}

void Engine::Update() {

}

void Engine::StartDraw() {
}

void Engine::Draw() {
	float currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	processInput(WindowManager::getWindow());

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shaderProgram->Activate();
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
	glm::mat4 view = camera.GetViewMatrix();
	shaderProgram->setMat4("view", view);

	// Projection, for perspective projection
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	shaderProgram->setMat4("projection", projection);

	//shaderProgram->setVec3("light.position", lightPos);
	shaderProgram->setVec3("cameraPos", camera.Position);

	// Material
	shaderProgram->setFloat("material.shininess", 32.0f);


	// directional light
	shaderProgram->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
	shaderProgram->setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	shaderProgram->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	shaderProgram->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
	// point light 1
	shaderProgram->setVec3("pointLights[0].position", pointLightPositions[0]);
	shaderProgram->setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	shaderProgram->setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	shaderProgram->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	shaderProgram->setFloat("pointLights[0].constant", 1.0f);
	shaderProgram->setFloat("pointLights[0].linear", 0.09f);
	shaderProgram->setFloat("pointLights[0].quadratic", 0.032f);
	// point light 2
	shaderProgram->setVec3("pointLights[1].position", pointLightPositions[1]);
	shaderProgram->setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	shaderProgram->setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
	shaderProgram->setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	shaderProgram->setFloat("pointLights[1].constant", 1.0f);
	shaderProgram->setFloat("pointLights[1].linear", 0.09f);
	shaderProgram->setFloat("pointLights[1].quadratic", 0.032f);
	// point light 3
	shaderProgram->setVec3("pointLights[2].position", pointLightPositions[2]);
	shaderProgram->setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	shaderProgram->setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
	shaderProgram->setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
	shaderProgram->setFloat("pointLights[2].constant", 1.0f);
	shaderProgram->setFloat("pointLights[2].linear", 0.09f);
	shaderProgram->setFloat("pointLights[2].quadratic", 0.032f);
	// point light 4
	shaderProgram->setVec3("pointLights[3].position", pointLightPositions[3]);
	shaderProgram->setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	shaderProgram->setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
	shaderProgram->setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
	shaderProgram->setFloat("pointLights[3].constant", 1.0f);
	shaderProgram->setFloat("pointLights[3].linear", 0.09f);
	shaderProgram->setFloat("pointLights[3].quadratic", 0.032f);
	// spotLight
	shaderProgram->setVec3("spotLight.position", camera.Position);
	shaderProgram->setVec3("spotLight.direction", camera.Front);
	shaderProgram->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	shaderProgram->setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	shaderProgram->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
	shaderProgram->setFloat("spotLight.constant", 1.0f);
	shaderProgram->setFloat("spotLight.linear", 0.09f);
	shaderProgram->setFloat("spotLight.quadratic", 0.032f);
	shaderProgram->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	shaderProgram->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

	//for (unsigned int i = 0; i < 10; i++)
	//{
	//	glm::mat4 model = glm::mat4(1.0f);
	//	model = glm::translate(model, cubePositions[i]);
	//	float angle = 20.0f * i;
	//	model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
	//	shaderProgram->setMat4("model", model);

	//	cubesMesh.Draw(shaderProgram, camera);  // Use your mesh instead
	//}
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	shaderProgram->setMat4("model", model);
	backPack->Draw(*shaderProgram, camera);

	// Draw light cube
	for (unsigned int i = 0; i < 4; i++)
	{
		glm::mat4 lightModel = glm::mat4(1.0f);
		lightModel = glm::translate(lightModel, pointLightPositions[i]);
		lightShader->setMat4("model", lightModel);  // Remove scale since light cube is already small

		lightCubeMesh->Draw(*lightShader, camera);  // Use your light mesh
	}

	glfwPollEvents();
}

void Engine::EndDraw() {
	glfwSwapBuffers(WindowManager::getWindow());
}

void Engine::Shutdown() {
	delete shaderProgram;
	delete backPack;
	delete lightShader;
	delete lightCubeMesh;
    std::cout << "[Engine] Shutdown complete" << std::endl;

}

bool Engine::IsRunning() {
	return !WindowManager::ShouldClose();
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.Position += cameraSpeed * camera.Front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.Position -= cameraSpeed * camera.Front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.Position -= glm::normalize(glm::cross(camera.Front, camera.Up)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.Position += glm::normalize(glm::cross(camera.Front, camera.Up)) * cameraSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}