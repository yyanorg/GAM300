#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include "Texture.h"
#include "ShaderClass.h"

enum class TextureType {
	DIFFUSE,
	SPECULAR,
	NORMAL,
	HEIGHT,
	AMBIENT_OCCLUSION,
	METALLIC,
	ROUGHNESS,
	EMISSIVE
};

class Material {
	Material();
	Material(const std::string& name);
	~Material() = default;

	// Basic Material Properties
	void SetAmbient(const glm::vec3 ambient);
	void SetDiffuse(const glm::vec3& diffuse);
	void SetSpecular(const glm::vec3& specular);
	void SetEmissive(const glm::vec3& emissive);
	void SetShininess(float shininess);
	void SetOpacity(float opacity);

	// PBR properties - Future
	void SetMetallic(float metallic);
	void SetRoughness(float roughness);
	void SetAO(float ao);

	// Texture Managment
	void SetTexture(TextureType type, std::shared_ptr<Texture> texture);
	std::shared_ptr<Texture> getTexture(TextureType type) const;
	bool hasTexture(TextureType type) const;
	void removeTexture(TextureType type);

	// Utility methods
	void setName(const std::string& name);
	const std::string& getName() const;

	// Apply material to shader
	void applyToShader(Shader& shader) const;

	// Static factory methods for common materials
	static std::shared_ptr<Material> createDefault();
	static std::shared_ptr<Material> createMetal(const glm::vec3& color);
	static std::shared_ptr<Material> createPlastic(const glm::vec3& color);
	static std::shared_ptr<Material> createWood();

private:
	std::string m_name;

	// Basic material properties
	glm::vec3 m_ambient{ 0.2f, 0.2f, 0.2f };
	glm::vec3 m_diffuse{ 0.8f, 0.8f, 0.8f };
	glm::vec3 m_specular{ 1.0f, 1.0f, 1.0f };
	glm::vec3 m_emissive{ 0.0f, 0.0f, 0.0f };
	float m_shininess{ 32.0f };
	float m_opacity{ 1.0f };

	// PBR properties
	float m_metallic{ 0.0f };
	float m_roughness{ 0.5f };
	float m_ao{ 1.0f };

	// Texture storage
	std::unordered_map<TextureType, std::shared_ptr<Texture>> m_textures;

	// Helper methods
	std::string textureTypeToString(TextureType type) const;
	void bindTextures(Shader& shader) const;

};