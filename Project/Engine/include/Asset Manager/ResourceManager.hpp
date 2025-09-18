#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <filesystem>
#include "GUID.hpp"
#include "Asset Manager/MetaFilesManager.hpp"
#include "Asset Manager/AssetManager.hpp"

class ResourceManager {
public:
	static ResourceManager& GetInstance() {
		static ResourceManager instance;
		return instance;
	}

	template <typename T>
	std::shared_ptr<T> GetResource(const std::string& assetPath) {
		auto& resourceMap = GetResourceMap<T>();
		std::filesystem::path filePathObj(assetPath);
		std::string filePath = filePathObj.generic_string();

		GUID_128 guid{};
		if (!MetaFilesManager::MetaFileExists(filePath)) {
			GUID_string guidStr = GUIDUtilities::GenerateGUIDString();
			guid = GUIDUtilities::ConvertStringToGUID128(guidStr);
		}
		else {
			guid = MetaFilesManager::GetGUID128FromAssetFile(filePath);
		}
		
		// Return a shared pointer to the resource (Texture, Model, etc.)
		auto it = resourceMap.find(guid);
		if (it != resourceMap.end()) {
			return it->second;
		}
		else {
			return LoadResource<T>(guid, filePath);
		}
	}

	//std::shared_ptr<Texture> GetTexture(const std::string& assetPath) {
	//	// Special handling for textures.
	//	auto& resourceMap = GetResourceMap<Texture>();
	//	std::filesystem::path filePathObj(assetPath);
	//	std::string filePath = filePathObj.generic_string();

	//	GUID_128 guid{};
	//	if (!MetaFilesManager::MetaFileExists(filePath)) {
	//		GUID_string guidStr = GUIDUtilities::GenerateGUIDString();
	//		guid = GUIDUtilities::ConvertStringToGUID128(guidStr);
	//	}
	//	else {
	//		guid = MetaFilesManager::GetGUID128FromAssetFile(filePath);
	//	}

	//	// Return a shared pointer to the Texture.
	//	auto it = resourceMap.find(guid);
	//	if (it != resourceMap.end()) {
	//		return it->second;
	//	}
	//	else {
	//		return LoadTexture(guid, filePath.c_str());
	//	}
	//}

	template <typename T>
	void UnloadResource(const std::string& assetPath) {
		// Implementation for unloading the resource
		auto& resourceMap = GetResourceMap<T>();
		if (MetaFilesManager::MetaFileExists(assetPath)) {
			GUID_128 guid = MetaFilesManager::GetGUID128FromAssetFile(assetPath);
			auto it = resourceMap.find(guid);
			if (it != resourceMap.end()) {
				resourceMap.erase(it);
			}
		}
	}

	template <typename T>
	void UnloadAllResourcesOfType() {
		GetResourceMap<T>().clear();
	}

private:
	ResourceManager() {};

	/**
	 * \brief Returns a singleton container for the asset type T.
	 *
	 * \tparam T The type of the assets.
	 * \return A reference to the map of assets for the specified type.
	 */
	template <typename T>
	std::unordered_map<GUID_128, std::shared_ptr<T>>& GetResourceMap() {
		static std::unordered_map<GUID_128, std::shared_ptr<T>> resourceMap;
		return resourceMap;
	}

	template <typename T>
	std::shared_ptr<T> LoadResource(const GUID_128& guid, const std::string& assetPath) {
		std::shared_ptr<T> resource = std::make_shared<T>();
		if (resource->LoadResource(assetPath)) {
			auto& resourceMap = GetResourceMap<T>();
			resourceMap[guid] = resource;
			std::cout << "[ResourceManager] Loaded resource for: " << assetPath << std::endl;
			return resource;
		}

		std::cerr << "[ResourceManager] ERROR: Failed to load resource: " << assetPath << std::endl;
		return nullptr;
	}

	//std::shared_ptr<Texture> LoadTexture(const GUID_128& guid, const char* assetPath) {
	//	std::shared_ptr<Texture> texture = std::make_shared<Texture>();
	//	if (texture->LoadResource(assetPath)) {
	//		auto& resourceMap = GetResourceMap<Texture>();
	//		resourceMap[guid] = texture;
	//		return texture;
	//	}

	//	std::cerr << "[ResourceManager] ERROR: Failed to load texture: " << assetPath << std::endl;
	//	return nullptr;
	//}
};