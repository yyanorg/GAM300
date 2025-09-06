#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include "GUID.hpp"
#include "MetaFilesManager.hpp"

class AssetManager {
public:
	static AssetManager& GetInstance() {
		static AssetManager instance;
		return instance;
	}

	template <typename T>
	std::shared_ptr<T> GetAsset(const std::string& filePath) {
		auto& assetMap = GetAssetMap<T>();

		if (!MetaFilesManager::MetaFileExists(filePath)) {
			return LoadAsset<T>(filePath);
		}
		else {
			GUID_128 guid = MetaFilesManager::GetGUID128FromAssetFile(filePath);
			auto it = assetMap.find(guid);
			if (it != assetMap.end()) {
				return it->second;
			}
			else {
				return Load<T>(filePath);
			}
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
	std::shared_ptr<T> LoadAsset(const std::string& filePath) {
		auto& assetMap = GetAssetMap<T>();

		// Ensure the asset has a .meta file and retrieve its GUID.
		GUID_128 guid{};
		if (!MetaFilesManager::MetaFileExists(filePath)) {
			guid = MetaFilesManager::GenerateMetaFile(filePath);
		}
		else {
			guid = MetaFilesManager::GetGUID128FromAssetFile(filePath);
		}

		// If the asset is not already loaded, load and store it using the GUID.
		if (assetMap.find(guid) == assetMap.end()) {
			std::shared_ptr<T> asset = std::make_shared<T>();
			if (!asset->LoadAsset(filePath)) {
				std::cerr << "[AssetManager] ERROR: Failed to load asset: " << filePath << std::endl;
				return nullptr;
			}

			assetMap[guid] = asset;
			return asset;
		}

		return assetMap[guid];
	}
};