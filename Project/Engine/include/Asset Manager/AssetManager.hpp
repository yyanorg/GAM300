#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <type_traits>
#include <filesystem>
#include "GUID.hpp"
#include "MetaFilesManager.hpp"

class AssetManager {
public:
	static AssetManager& GetInstance() {
		static AssetManager instance;
		return instance;
	}

	template <typename T>
	std::shared_ptr<T> GetAsset(const std::string& filePathStr) {
		static_assert(!std::is_same_v<T, Texture>,
			"Calling AssetManager::GetInstance().GetAsset() to get a texture is forbidden. Use GetTexture() instead.");

		auto& assetMap = GetAssetMap<T>();
		std::filesystem::path filePathObj(filePathStr);
		std::string filePath = filePathObj.generic_string();

		GUID_128 guid{};
		if (!MetaFilesManager::MetaFileExists(filePath)) {
			guid = MetaFilesManager::GenerateMetaFile(filePath);
		}
		else {
			guid = MetaFilesManager::GetGUID128FromAssetFile(filePath);
		}

		auto it = assetMap.find(guid);
		if (it != assetMap.end()) {
			return it->second;
		}
		else {
			return CompileAssetToResource<T>(guid, filePath);
		}
	}

	std::shared_ptr<Texture> GetTexture(std::string filePath, std::string texType, GLuint slot, GLenum pixelType) {
		auto& assetMap = GetAssetMap<Texture>();

		GUID_128 guid{};
		if (!MetaFilesManager::MetaFileExists(filePath)) {
			guid = MetaFilesManager::GenerateMetaFile(filePath);
		}
		else {
			guid = MetaFilesManager::GetGUID128FromAssetFile(filePath);
		}

		auto it = assetMap.find(guid);
		if (it != assetMap.end()) {
			return it->second;
		}
		else {
			return CompileTextureToResource(guid, filePath.c_str(), texType.c_str(), slot);
		}
	}

	template <typename T>
	void UnloadAsset(const std::string& filePath) {
		auto& assetMap = GetAssetMap<T>();
		if (MetaFilesManager::MetaFileExists(filePath)) {
			GUID_128 guid = MetaFilesManager::GetGUID128FromAssetFile(filePath);
			auto it = assetMap.find(guid);
			if (it != assetMap.end()) {
				assetMap.erase(it);
			}
		}
	}

	template <typename T>
	void UnloadAllAssetsOfType() {
		GetAssetMap<T>().clear();
	}

private:
	AssetManager() {};

	/**
	 * \brief Returns a singleton container for the asset type T.
	 *
	 * \tparam T The type of the assets.
	 * \return A reference to the map of assets for the specified type.
	 */
	template <typename T>
	std::unordered_map<GUID_128, std::shared_ptr<T>>& GetAssetMap() {
		static std::unordered_map<GUID_128, std::shared_ptr<T>> assetMap;
		return assetMap;
	}

	template <typename T>
	std::shared_ptr<T> CompileAssetToResource(GUID_128 guid, const std::string& filePath) {
		auto& assetMap = GetAssetMap<T>();

		// If the asset is not already loaded, load and store it using the GUID.
		if (assetMap.find(guid) == assetMap.end()) {
			std::shared_ptr<T> asset = std::make_shared<T>();
			if (!asset->CompileToResource(filePath)) {
				std::cerr << "[AssetManager] ERROR: Failed to load asset: " << filePath << std::endl;
				return nullptr;
			}

			assetMap[guid] = asset;
			return asset;
		}

		return assetMap[guid];
	}

	std::shared_ptr<Texture> CompileTextureToResource(GUID_128 guid, const char* filePath, const char* texType, GLuint slot) {
		auto& assetMap = GetAssetMap<Texture>();

		// If the asset is not already loaded, load and store it using the GUID.
		if (assetMap.find(guid) == assetMap.end()) {
			std::shared_ptr<Texture> asset = std::make_shared<Texture>( texType, slot);
			if (!asset->CompileToResource(filePath)) {
				std::cerr << "[AssetManager] ERROR: Failed to compile asset: " << filePath << std::endl;
				return nullptr;
			}

			assetMap[guid] = asset;
			return asset;
		}

		return assetMap[guid];
	}
};