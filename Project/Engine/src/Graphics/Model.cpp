#include "pch.h"
#include "Graphics/Model.h"
#include "Graphics/TextureManager.h"
#include <iostream>
#include "Asset Manager/AssetManager.hpp"

bool Model::LoadAsset(const std::string& path) 
{
	Assimp::Importer importer;
	// The function expects a file path and several post-processing options as its second argument
	// aiProcess_Triangulate tells Assimp that if the model does not (entirely) consist of triangles, it should transform all the model's primitive shapes to triangles first.
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR:ASSIMP:: " << importer.GetErrorString() << std::endl;
		return false;
	}

	directory = path.substr(0, path.find_last_of('/'));

	// Recursive function
	processNode(scene->mRootNode, scene);

	return true;
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	// Process each mesh in this node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.emplace_back(processMesh(mesh, scene));
	}

	// Process children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}

}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<std::shared_ptr<Texture>> textures;

    // Process vertices (same as before)
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        // Position
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        // Normals
        if (mesh->HasNormals())
        {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        // Texture coordinates
        if (mesh->mTextureCoords[0])
        {
            vertex.texUV.x = mesh->mTextureCoords[0][i].x;
            vertex.texUV.y = mesh->mTextureCoords[0][i].y;
        }
        else
        {
            vertex.texUV = glm::vec2(0.f, 0.f);
        }

        vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
        vertices.push_back(vertex);
    }

    // Process indices (same as before)
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Create material from Assimp material
    std::shared_ptr<Material> material = nullptr;
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* assimpMaterial = scene->mMaterials[mesh->mMaterialIndex];

        // Create new material
        aiString materialName;
        assimpMaterial->Get(AI_MATKEY_NAME, materialName);
        material = std::make_shared<Material>(materialName.C_Str());

        // Load material properties
        aiColor3D color;

        // Ambient
        if (assimpMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) 
        {
            material->SetAmbient(glm::vec3(color.r, color.g, color.b));
        }

        // Diffuse
        if (assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) 
        {
            material->SetDiffuse(glm::vec3(color.r, color.g, color.b));
        }

        // Specular
        if (assimpMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) 
        {
            material->SetSpecular(glm::vec3(color.r, color.g, color.b));
        }

        // Emissive
        if (assimpMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) 
        {
            material->SetEmissive(glm::vec3(color.r, color.g, color.b));
        }

        // Shininess
        float shininess;
        if (assimpMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) 
        {
            material->SetShininess(shininess);
        }

        // Opacity
        float opacity;
        if (assimpMaterial->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) 
        {
            material->SetOpacity(opacity);
        }

        // Load textures and assign to material

        // Diffuse textures
        std::vector<std::shared_ptr<Texture>> diffuseMaps = loadMaterialTexture(assimpMaterial, aiTextureType_DIFFUSE, "diffuse");
        if (!diffuseMaps.empty()) {
            material->SetTexture(TextureType::DIFFUSE, diffuseMaps[0]);
        }

        // Specular textures
        std::vector<std::shared_ptr<Texture>> specularMaps = loadMaterialTexture(assimpMaterial, aiTextureType_SPECULAR, "specular");
        if (!specularMaps.empty()) 
        {
            material->SetTexture(TextureType::SPECULAR, specularMaps[0]);
        }

        // Normal maps
        std::vector<std::shared_ptr<Texture>> normalMaps = loadMaterialTexture(assimpMaterial, aiTextureType_NORMALS, "normal");
        if (!normalMaps.empty()) 
        {
            material->SetTexture(TextureType::NORMAL, normalMaps[0]);
        }

        // Height maps
        std::vector<std::shared_ptr<Texture>> heightMaps = loadMaterialTexture(assimpMaterial, aiTextureType_HEIGHT, "height");
        if (!heightMaps.empty()) {
            material->SetTexture(TextureType::HEIGHT, heightMaps[0]);
        }

        // Keep old texture list for backward compatibility
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    // If no material was created, use a default one
    if (!material) 
    {
        material = Material::createDefault();
    }

    return Mesh(vertices, indices, material);
}

std::vector<std::shared_ptr<Texture>> Model::loadMaterialTexture(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	typeName;
	std::vector<std::shared_ptr<Texture>> textures;
	//TextureManager& textureManager = TextureManager::getInstance();

	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string texturePath = directory + '/' + str.C_Str();

		// Use the asset manager
		auto texture = AssetManager::GetInstance().GetTexture(texturePath, typeName, -1, GL_UNSIGNED_BYTE);
		//auto texture = textureManager.loadTexture(texturePath, typeName);
		if (texture) 
		{
			textures.push_back(texture); // Dereference shared_ptr
		}
	}

	return textures;
}

void Model::Draw(Shader& shader, Camera& camera)
{
	for (auto& mesh : meshes)
	{
		mesh.Draw(shader, camera);
	}
}