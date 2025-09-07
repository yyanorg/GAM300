#include "pch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Graphics/Renderer.hpp"
#include "Graphics/LightManager.hpp"

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

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

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
Renderer& renderer = Renderer::getInstance();
std::shared_ptr<Model> backpackModel;
std::shared_ptr<Shader> shader;
//----------------LIGHT-------------------
std::shared_ptr<Shader> lightShader;
std::shared_ptr<Mesh> lightCubeMesh;

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

	glfwSetCursorPosCallback(WindowManager::getWindow(), mouse_callback);
	glfwSetScrollCallback(WindowManager::getWindow(), scroll_callback);
	glfwSetInputMode(WindowManager::getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	

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

	
	renderer.Initialise(SCR_WIDTH, SCR_HEIGHT);

	// Loads model
	backpackModel = std::make_shared<Model>("Resources/Models/backpack/backpack.obj");
	shader = std::make_shared<Shader>("Resources/Shaders/default.vert", "Resources/Shaders/default.frag");

	// Creates light
	lightShader = std::make_shared<Shader>("Resources/Shaders/light.vert", "Resources/Shaders/light.frag");
	std::vector<std::shared_ptr<Texture>> emptyTextures = {};
	lightCubeMesh = std::make_shared<Mesh>(lightVertices, lightIndices, emptyTextures);

	// Sets camera
	renderer.SetCamera(&camera);

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

	// TRIED TO PUT THIS IN ENGINE::UPDATE(), DONT BOTHER ITS TOO MANY BUGS NOW

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

		renderer.BeginFrame();
		renderer.Clear();

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, 0.0f));
		transform = glm::scale(transform, glm::vec3(0.1f, 0.1f, 0.1f));
		renderer.Submit(backpackModel, transform, shader);

		renderer.Render();
		renderer.EndFrame();

		// Draw light cube
		LightManager& lightManager = LightManager::getInstance();
		const auto& pointLights = lightManager.getPointLights();
		for (unsigned int i = 0; i < pointLights.size(); i++) 
		{
			glm::mat4 lightModel = glm::mat4(1.0f);
			lightModel = glm::translate(lightModel, pointLights[i].position);
			lightShader->setMat4("model", lightModel);
			lightCubeMesh->Draw(*lightShader, camera);
		}

		glfwPollEvents();
}

void Engine::EndDraw() {
	glfwSwapBuffers(WindowManager::getWindow());
}

void Engine::Shutdown() {
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