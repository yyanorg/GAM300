#pragma once
#include "Graphics/Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <algorithm>
#include "Asset Manager/Asset.hpp"

class Model : public IAsset {
public:
	std::vector<Mesh> meshes;
	std::string directory;

	//Model(const std::string& filePath);
	bool CompileToResource(const std::string& path) override;
	bool CompileMesh(aiMesh* mesh, const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, std::shared_ptr<Material> mat);
	void Draw(Shader& shader, const Camera& camera);

private:
	//void loadModel(const std::string& path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<std::shared_ptr<Texture>> LoadMaterialTexture(aiMaterial* mat, aiTextureType type, std::string typeName);

};