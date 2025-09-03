#pragma once

#include "VAO.h"
#include "EBO.h"
#include "Camera.h"
#include "Texture.h"

class Mesh {
public:
	std::vector<Vertex> vertices; 
	std::vector<GLuint> indices; 
	std::vector<std::shared_ptr<Texture>> textures;

	Mesh() {};
	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<std::shared_ptr<Texture>>& textures);
	~Mesh();
	void Draw(Shader& shader, Camera& camera);

	Mesh(const Mesh& other) = delete;  // Prevent copying
	Mesh& operator=(const Mesh& other) = delete;  // Prevent assignment

	Mesh(Mesh&& other) noexcept
		: vertices(std::move(other.vertices)),
		indices(std::move(other.indices)),
		textures(std::move(other.textures)),
		vao(std::move(other.vao)) {}
private:
	VAO vao;

};