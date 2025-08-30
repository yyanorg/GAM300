#include "pch.h"

#include "Graphics/Mesh.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture>& textures) : vertices(vertices), indices(indices), textures(textures)
{
	// Generates Vertex Array Object and binds it
	vao.Bind();

	VBO vbo(vertices);
	vbo.Bind();

	EBO ebo(indices);

	// Position
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0); // Compiler knows the exact size of your Vertex struct (including any padding) no need 11 * sizeof(float)
	// Normal
	vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	// Color
	vao.LinkAttrib(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	// Texture
	vao.LinkAttrib(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));

	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}

Mesh::~Mesh()
{
	vao.Delete();
}

void Mesh::Draw(Shader& shader, Camera& camera)
{
	shader.Activate();
	vao.Bind();

	unsigned int numDiffuse = 0, numSpecular = 0;

	// UNDERSTAND THIS FOR LOOP
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		std::string num;
		std::string type = textures[i].type;

		if (type == "diffuse")
		{
			num = std::to_string(numDiffuse++);
		}
		else if (type == "specular")
		{
			num = std::to_string(numSpecular++);
		}
		shader.setInt(("material." + type + num).c_str(), i);
		textures[i].Bind();
	}
	// HOW ARE THE DIFFUSE AND SPECULAR MAP TEXTURE BEING ASSIGNED IN THE FRAGMENT SHADER

	glm::mat4 view = camera.GetViewMatrix();
	shader.setMat4("view", view);

	// Projection, for perspective projection
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	shader.setMat4("projection", projection);

	//shaderProgram.setVec3("light.position", lightPos);
	shader.setVec3("cameraPos", camera.Position);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}
