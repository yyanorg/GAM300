#include "pch.h"

#include "Graphics/VAO.h"

VAO::VAO()
{
	//glGenVertexArrays(1, &ID);
}

void VAO::LinkAttrib(VBO VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset)
{
	VBO.Bind();
	glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);
	VBO.Unbind();
}

void VAO::Bind()
{
	if (ID == 0) {
		glGenVertexArrays(1, &ID);  // Temp workaround for refactoring - pulled out Mesh that uses VAO, but need default constructor, but
		// genVertex will crash if glfw isn't init yet, so we will init only when binding
	}

	glBindVertexArray(ID);
}

void VAO::Unbind()
{
	glBindVertexArray(0);
}

void VAO::Delete()
{
	glDeleteVertexArrays(1, &ID);
}
