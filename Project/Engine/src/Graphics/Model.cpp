#include "pch.h"
#include "Graphics/Model.h"
#include "Graphics/TextureManager.h"
#include <iostream>
#include "Asset Manager/AssetManager.hpp"

//Model::Model(const std::string& filePath)
//{
//	//loadModel(filePath);
//	/*std::cout << "Model loaded with " << meshes.size() << " meshes" << std::endl;
//	for (int i = 0; i < meshes.size(); i++)
//	{
//		std::cout << "Mesh " << i << ": " << meshes[i].vertices.size() << " vertices, "
//			<< meshes[i].indices.size() << " indices" << std::endl;
//	}*/
//}

bool Model::LoadAsset(const std::string& path) {
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

//void Model::loadModel(const std::string& path)
//{
//	Assimp::Importer importer;
//	// The function expects a file path and several post-processing options as its second argument
//	// aiProcess_Triangulate tells Assimp that if the model does not (entirely) consist of triangles, it should transform all the model's primitive shapes to triangles first.
//	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
//
//	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
//	{
//		std::cout << "ERROR:ASSIMP:: " << importer.GetErrorString() << std::endl;
//		return;
//	}
//
//	directory = path.substr(0, path.find_last_of('/'));
//
//	// Recursive function
//	processNode(scene->mRootNode, scene);
//}

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

	// Process vertices
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

		// Texture
		if (mesh->mTextureCoords[0]) // why only check the first?
		{
			vertex.texUV.x = mesh->mTextureCoords[0][i].x;
			vertex.texUV.y = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			vertex.texUV = glm::vec2(0.f, 0.f);
		}

		// Color (default to white)
		vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);

		vertices.push_back(vertex);
	}

	// Process ALL faces (not just the first 5)
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// Process materials
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// Load diffuse textures - now returns shared_ptr vector
		std::vector<std::shared_ptr<Texture>> diffuseMaps = loadMaterialTexture(material, aiTextureType_DIFFUSE, "diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		// Load specular textures - now returns shared_ptr vector  
		std::vector<std::shared_ptr<Texture>> specularMaps = loadMaterialTexture(material, aiTextureType_SPECULAR, "specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, textures);
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