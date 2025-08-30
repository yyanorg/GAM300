#pragma once

#include "pch.h"

#include <glad/glad.h>

class EBO {
public:
	GLuint ID{};
	EBO(std::vector<GLuint>& indices);

	void Bind();
	void Unbind();
	void Delete();
};