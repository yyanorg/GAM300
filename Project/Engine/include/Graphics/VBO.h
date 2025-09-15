#pragma once
#include "OpenGL.h"
#include <glm/glm.hpp>
#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texUV;
};

// VBO stores multiple vertices on the GPU memory
class VBO {
public:
	GLuint ID{};
	VBO(std::vector<Vertex>& vertices);

	void Bind();
	void Unbind();
	void Delete();
};