#include "pch.h"

#include "Graphics/Mesh.h"

// Removed hardcoded screen dimensions - matrices are now handled by GraphicsManager

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<std::shared_ptr<Texture>>& textures) : vertices(vertices), indices(indices), textures(textures)
{
	setupMesh();
}

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices, std::shared_ptr<Material> mat) : vertices(vertices), indices(indices), material(mat)
{
	setupMesh();
}

Mesh::~Mesh()
{
	vao.Delete();
}

void Mesh::setupMesh()
{
	// Generates Vertex Array Object and binds it
	vao.Bind();

	VBO vbo(vertices);
	vbo.Bind();

	EBO ebo(indices);

	// Position
	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void *)0); // Compiler knows the exact size of your Vertex struct (including any padding) no need 11 * sizeof(float)
	// Normal
	vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void *)(3 * sizeof(float)));
	// Color
	vao.LinkAttrib(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void *)(6 * sizeof(float)));
	// Texture
	vao.LinkAttrib(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void *)(9 * sizeof(float)));

	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}

void Mesh::Draw(Shader& shader, const Camera& camera)
{
	vao.Delete();
	void Mesh::Draw(Shader & shader, const Camera &camera)
	{
		shader.Activate();
		vao.Bind();

    // Only set camera position for lighting calculations
    // Note: view and projection matrices are already set by GraphicsManager::SetupMatrices()
    shader.setVec3("cameraPos", camera.Position);

				std::string num;
				std::string type = textures[i]->type;

				if (type == "diffuse")
				{
					num = std::to_string(numDiffuse++);
				}
				else if (type == "specular")
				{
					num = std::to_string(numSpecular++);
				}

				glActiveTexture(GL_TEXTURE0 + textureUnit);
				textures[i]->Bind();
				shader.setInt(("material." + type + num).c_str(), textureUnit);
				textureUnit++;
			}
		}

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}

	void Mesh::Draw(Shader & shader, Camera & camera)
	{
		shader.Activate();
		vao.Bind();

		// Dynamic texture binding
		unsigned int textureUnit = 0;
		unsigned int numDiffuse = 0, numSpecular = 0;

		for (unsigned int i = 0; i < textures.size() && textureUnit < 16; i++)
		{
			if (!textures[i])
				continue;

			std::string num;
			std::string type = textures[i]->type;

			if (type == "diffuse")
			{
				num = std::to_string(numDiffuse++);
			}
			else if (type == "specular")
			{
				num = std::to_string(numSpecular++);
			}

			// Bind texture to current unit
			glActiveTexture(GL_TEXTURE0 + textureUnit);
			textures[i]->Bind();

			// Set shader uniform
			shader.setInt(("material." + type + num).c_str(), textureUnit);

			textureUnit++;
		}
		// HOW ARE THE DIFFUSE AND SPECULAR MAP TEXTURE BEING ASSIGNED IN THE FRAGMENT SHADER

		glm::mat4 view = camera.GetViewMatrix();
		shader.setMat4("view", view);

		// Projection, for perspective projection
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		shader.setMat4("projection", projection);

		// shaderProgram.setVec3("light.position", lightPos);
		shader.setVec3("cameraPos", camera.Position);

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}
