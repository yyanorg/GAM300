#include "pch.h"
#include "Graphics/TextureManager.h"
#include <filesystem>

int TextureManager::nextTextureUnit = 0;

TextureManager& TextureManager::getInstance()
{
	static TextureManager instance;
	return instance;
}

std::shared_ptr<Texture> TextureManager::loadTexture(const std::string& filepath, const std::string& type)
{
	auto it = textureCache.find(filepath);
	if (it != textureCache.end())
	{
		return it->second;
	}

	if (!std::filesystem::exists(filepath))
	{
		std::cerr << "[TextureManager] ERROR: Texture file not found: " << filepath << std::endl;
		return nullptr;
	}

	GLenum format = getFormatFromExtension(filepath);

	try {
		auto texture = std::make_shared<Texture>(filepath.c_str(), type.c_str(), nextTextureUnit++, format, GL_UNSIGNED_BYTE);
		
		textureCache[filepath] = texture;
		std::cout << "[TextureManager] Successfully loaded texture: " << filepath << " (Unit: " << (nextTextureUnit - 1) << ")" << std::endl;

		return texture;
	}
	catch (const std::exception& e)
	{
		std::cerr << "[TextureManager] ERROR loading texture " << filepath << ": " << e.what() << std::endl;
		return nullptr;
	}
	return std::shared_ptr<Texture>();
}

std::shared_ptr<Texture> TextureManager::getTexture(const std::string& filepath)
{
	auto it = textureCache.find(filepath);
	if (it != textureCache.end())
	{
		return it->second;
	}

	return nullptr;
}

bool TextureManager::isLoaded(const std::string& filepath) const
{
	return textureCache.find(filepath) != textureCache.end();
}

void TextureManager::unloadTexture(const std::string& filepath)
{
	auto it = textureCache.find(filepath);
	if (it != textureCache.end())
	{
		std::cout << "[TextureManager] Unloading texture: " << filepath << std::endl;
		textureCache.erase(filepath);
	}
}

void TextureManager::clearCache()
{
	std::cout << "[TextureManager] Clearing texture cache (" << textureCache.size() << " textures)" << std::endl;
	textureCache.clear();
}

size_t TextureManager::getCacheSize() const
{
	return textureCache.size();
}

void TextureManager::printCacheStats() const
{
	std::cout << "[TextureManager] Cache Stats:" << std::endl;
	std::cout << "  - Cached textures: " << textureCache.size() << std::endl;
	std::cout << "  - Next texture unit: " << nextTextureUnit << std::endl;

	if (!textureCache.empty()) 
	{
		std::cout << "  - Loaded textures:" << std::endl;
		for (const auto& [path, texture] : textureCache) 
		{
			std::cout << "    * " << path << std::endl;
		}
	}
}

GLenum TextureManager::getFormatFromExtension(const std::string& filepath)
{
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
