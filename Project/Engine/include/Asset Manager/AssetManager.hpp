#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <type_traits>
#include <filesystem>
#include "GUID.hpp"
#include "MetaFilesManager.hpp"
#include "Asset Manager/AssetMeta.hpp"
#include <Graphics/Model/Model.h>
#include "Graphics/TextRendering/Font.hpp"

class AssetManager {
public:
	static AssetManager& GetInstance() {
		static AssetManager instance;
		return instance;
	}

	bool CompileAsset(const std::string& filePathStr) {
		std::filesystem::path filePathObj(filePathStr);
		std::string extension = filePathObj.extension().string();
		if (textureExtensions.find(extension) != textureExtensions.end()) {
			return CompileTexture(filePathStr, "diffuse", -1);
		}
		//else if (audioExtensions.find(extension) != audioExtensions.end()) {
		//	return CompileAsset<Audio>(filePathStr);
		//}
		else if (fontExtensions.find(extension) != fontExtensions.end()) {
			return CompileAsset<Font>(filePathStr);
		}
		else if (modelExtensions.find(extension) != modelExtensions.end()) {
			return CompileAsset<Model>(filePathStr);
		}
		else if (shaderExtensions.find(extension) != shaderExtensions.end()) {
			return CompileAsset<Shader>(filePathStr);
		}
		else {
			std::cerr << "[AssetManager] ERROR: Unsupported asset extension: " << extension << std::endl;
			return false;
		}
	}

	template <typename T>
	bool CompileAsset(const std::string& filePathStr) {
		static_assert(!std::is_same_v<T, Texture>,
			"Calling AssetManager::GetInstance().GetAsset() to compile a texture is forbidden. Use CompileTexture() instead.");

		std::filesystem::path filePathObj(filePathStr);
		std::string filePath;
		if (std::is_same_v<T, Shader>) {
			filePath = (filePathObj.parent_path() / filePathObj.stem()).generic_string();
		}
		else filePath = filePathObj.generic_string();

		GUID_128 guid{};
		if (!MetaFilesManager::MetaFileExists(filePath) || !MetaFilesManager::MetaFileUpdated(filePath)) {
			GUID_string guidStr = GUIDUtilities::GenerateGUIDString();
			guid = GUIDUtilities::ConvertStringToGUID128(guidStr);
		}
		else {
			guid = MetaFilesManager::GetGUID128FromAssetFile(filePath);
		}

		auto it = assetMetaMap.find(guid);
		if (it != assetMetaMap.end()) {
			return true;
		}
		else {
			return CompileAssetToResource<T>(guid, filePath);
		}
	}

	bool CompileTexture(std::string filePath, std::string texType, GLint slot) {
		GUID_128 guid{};
		if (!MetaFilesManager::MetaFileExists(filePath) || !MetaFilesManager::MetaFileUpdated(filePath)) {
			GUID_string guidStr = GUIDUtilities::GenerateGUIDString();
			guid = GUIDUtilities::ConvertStringToGUID128(guidStr);
		}
		else {
			guid = MetaFilesManager::GetGUID128FromAssetFile(filePath);
		}

		auto it = assetMetaMap.find(guid);
		if (it != assetMetaMap.end()) {
			return true;
		}
		else {
			return CompileTextureToResource(guid, filePath.c_str(), texType.c_str(), slot);
		}
	}

	bool IsAssetCompiled(GUID_128 guid) {
		return assetMetaMap.find(guid) != assetMetaMap.end();
	}

	//void UnloadAsset(const std::string& filePath) {
	//	if (MetaFilesManager::MetaFileExists(filePath)) {
	//		GUID_128 guid = MetaFilesManager::GetGUID128FromAssetFile(filePath);
	//		auto it = assetMetaMap.find(guid);
	//		if (it != assetMetaMap.end()) {
	//			assetMetaMap.erase(it);
	//		}
	//	}
	//}

	void UnloadAllAssets() {
		assetMetaMap.clear();
	}

	void InitializeSupportedExtensions() {
		supportedExtensions.insert(textureExtensions.begin(), textureExtensions.end());
		supportedExtensions.insert(audioExtensions.begin(), audioExtensions.end());
		supportedExtensions.insert(fontExtensions.begin(), fontExtensions.end());
		supportedExtensions.insert(modelExtensions.begin(), modelExtensions.end());
		supportedExtensions.insert(shaderExtensions.begin(), shaderExtensions.end());
	}

	std::unordered_set<std::string>& GetSupportedExtensions() {
		return supportedExtensions;
	}

	const std::unordered_set<std::string>& GetShaderExtensions() const {
		return shaderExtensions;
	}

private:
	AssetManager() {};

	std::unordered_map<GUID_128, std::shared_ptr<AssetMeta>> assetMetaMap;

	// Supported extensions
	const std::unordered_set<std::string> textureExtensions = { ".png", ".PNG", ".jpg", ".JPG", ".jpeg", ".JPEG", ".bmp", ".BMP" };
	const std::unordered_set<std::string> audioExtensions = { ".wav", ".ogg" };
	const std::unordered_set<std::string> fontExtensions = { ".ttf" };
	const std::unordered_set<std::string> modelExtensions = { ".obj", ".fbx" };
	const std::unordered_set<std::string> shaderExtensions = { ".vert", ".frag" };
	std::unordered_set<std::string> supportedExtensions;

	///**
	// * \brief Returns a singleton container for the asset type T.
	// *
	// * \tparam T The type of the assets.
	// * \return A reference to the map of assets for the specified type.
	// */
	//template <typename T>
	//std::unordered_map<GUID_128, std::shared_ptr<T>>& GetAssetMap() {
	//	static std::unordered_map<GUID_128, std::shared_ptr<T>> assetMap;
	//	return assetMap;
	//}

	template <typename T>
	bool CompileAssetToResource(GUID_128 guid, const std::string& filePath) {
		// If the asset is not already loaded, load and store it using the GUID.
		if (assetMetaMap.find(guid) == assetMetaMap.end()) {
			std::shared_ptr<T> asset = std::make_shared<T>();
			std::string compiledPath = asset->CompileToResource(filePath);
			if (compiledPath.empty()) {
				std::cerr << "[AssetManager] ERROR: Failed to compile asset: " << filePath << std::endl;
				return false;
			}

			std::shared_ptr<AssetMeta> assetMeta = asset->GenerateBaseMetaFile(guid, filePath, compiledPath);
			assetMetaMap[guid] = assetMeta;
			std::cout << "[AssetManager] Compiled asset: " << filePath << " to " << compiledPath << std::endl;
			return true;
		}

		return true;
	}

	bool CompileTextureToResource(GUID_128 guid, const char* filePath, const char* texType, GLint slot) {
		// If the asset is not already loaded, load and store it using the GUID.
		if (assetMetaMap.find(guid) == assetMetaMap.end()) {
			Texture texture{ texType, slot };
			std::string compiledPath = texture.CompileToResource(filePath);
			if (compiledPath.empty()) {
				std::cerr << "[AssetManager] ERROR: Failed to compile asset: " << filePath << std::endl;
				return false;
			}

			std::shared_ptr<AssetMeta> assetMeta = texture.GenerateBaseMetaFile(guid, filePath, compiledPath);
			assetMeta = texture.ExtendMetaFile(filePath, assetMeta);
			assetMetaMap[guid] = assetMeta;
			std::cout << "[AssetManager] Compiled asset: " << filePath << " to " << compiledPath << std::endl;
			return true;
		}

		return true;
	}
};