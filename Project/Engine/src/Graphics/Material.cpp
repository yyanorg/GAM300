#include "pch.h"
#include "Graphics/Material.hpp"

Material::Material() : m_name("DefaultMaterial") {
}

Material::Material(const std::string& name) : m_name(name) {
}

void Material::SetAmbient(const glm::vec3 ambient)
{
}

void Material::SetDiffuse(const glm::vec3& diffuse)
{
}

void Material::SetSpecular(const glm::vec3& specular)
{
}

void Material::SetEmissive(const glm::vec3& emissive)
{
}

void Material::SetShininess(float shininess)
{
}

void Material::SetOpacity(float opacity)
{
}

void Material::SetMetallic(float metallic)
{
}

void Material::SetRoughness(float roughness)
{
}

void Material::SetAO(float ao)
{
}

void Material::SetTexture(TextureType type, std::shared_ptr<Texture> texture)
{
}

std::shared_ptr<Texture> Material::getTexture(TextureType type) const
{
	return std::shared_ptr<Texture>();
}

bool Material::hasTexture(TextureType type) const
{
	return false;
}

void Material::removeTexture(TextureType type)
{
}

void Material::setName(const std::string& name)
{
}

const std::string& Material::getName() const
{
	// TODO: insert return statement here
}

void Material::applyToShader(Shader& shader) const
{
}

std::shared_ptr<Material> Material::createDefault()
{
	return std::shared_ptr<Material>();
}

std::shared_ptr<Material> Material::createMetal(const glm::vec3& color)
{
	return std::shared_ptr<Material>();
}

std::shared_ptr<Material> Material::createPlastic(const glm::vec3& color)
{
	return std::shared_ptr<Material>();
}

std::shared_ptr<Material> Material::createWood()
{
	return std::shared_ptr<Material>();
}

std::string Material::textureTypeToString(TextureType type) const
{
	return std::string();
}

void Material::bindTextures(Shader& shader) const
{
}
