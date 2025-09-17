#include "pch.h"

#include "Graphics/Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Graphics/stb_image.h"
#include "GLI/gli.hpp"
#include <filesystem>

Texture::Texture() : ID(0), type(type), unit(0) {}

Texture::Texture(const char* texType, GLuint slot)
{
	// Assigns the type of the texture ot the texture object
	type = texType;
	unit = slot; // Store the slot, but don't bind if it's -1
}

bool Texture::CompileToResource(const std::string& path) {
	// Stores the width, height, and the number of color channels of the image
	int widthImg, heightImg, numColCh;
	// Flips the image so it appears right side up
	stbi_set_flip_vertically_on_load(true);
	// Reads the image from a file and stores it in bytes
	unsigned char* bytes = stbi_load(path.c_str(), &widthImg, &heightImg, &numColCh, 0);

	// Dynamically choose the appropriate format.
	gli::format gliFormat;
	switch (numColCh)
	{
		case 1:
			gliFormat = gli::FORMAT_R8_UNORM_PACK8;
			break;
		case 2:
			gliFormat = gli::FORMAT_RG8_UNORM_PACK8;
			break;
		case 3:
			gliFormat = gli::FORMAT_RGB8_UNORM_PACK8;
			break;
		case 4:
			gliFormat = gli::FORMAT_RGBA8_UNORM_PACK8;
			break;
		default:
			stbi_image_free(bytes);
			return false; // Unsupported format
	}

	// Create GLI texture object.
	gli::texture2d texture(gliFormat, glm::uvec2(widthImg, heightImg), 1);
	std::memcpy(texture.data(), bytes, widthImg * heightImg * numColCh);

	// Save the texture to a DDS file.
	std::filesystem::path p(path);
	gli::save(texture, p.stem().string() + ".dds");

	// Free original image data.
	stbi_image_free(bytes);
}

bool Texture::LoadResource(const std::string& path) {
	gli::texture texture = gli::load(path);
	void* bytes = texture.data();
	int widthImg = texture.extent().x;
	int heightImg = texture.extent().y;
	
	gli::gl GL(gli::gl::PROFILE_GL33);
	gli::gl::format const format = GL.translate(texture.format(), texture.swizzles());
	GLenum target = texture.target();

	// Generates an OpenGL texture object
	glGenTextures(1, &ID);

	//// Assigns the texture to a Texture Unit
	//glActiveTexture(GL_TEXTURE0 + slot);
	//unit = slot;
	//glBindTexture(GL_TEXTURE_2D, ID);

	// Only bind to unit if slot is valid
	if (unit >= 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(target, ID);
	}

	// Configures the type of algorithm that is used to make the image smaller or bigger
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Configures the way the texture repeats (if it does at all)
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture.levels() - 1));
	glTexParameteriv(target, GL_TEXTURE_SWIZZLE_RGBA, &format.Swizzles[0]);

	// Extra lines in case you choose to use GL_CLAMP_TO_BORDER
	// float flatColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	// glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, flatColor);

	// Assigns the image to the OpenGL Texture object
	glTexImage2D(target, 0, format.Internal, widthImg, heightImg, 0, format.External, format.Type, bytes);
	// Generates MipMaps
	glGenerateMipmap(target);

	// Unbinds the OpenGL Texture object so that it can't accidentally be modified
	glBindTexture(target, 0);

	return true;
}

GLenum Texture::GetFormatFromExtension(const std::string& filepath) {
	std::string extension = filepath.substr(filepath.find_last_of('.'));

	if (extension == ".png" || extension == ".PNG")
	{
		return GL_RGBA;
	}
	else if (extension == ".jpg" || extension == ".jpeg" || extension == ".JPG" || extension == ".JPEG")
	{
		return GL_RGB;
	}
	else if (extension == ".bmp" || extension == ".BMP")
	{
		return GL_RGB;
	}
	else
	{
		// Default to RGB for unknown formats
		return GL_RGB;
	}
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit)
{
	// Shader needs to be activated before changing the value of a uniform
	shader.Activate();
	// Sets the value of the uniform
	shader.setInt(uniform, unit);
}

void Texture::Bind()
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Delete()
{
	glDeleteTextures(1, &ID);
}