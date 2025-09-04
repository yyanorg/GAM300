#pragma once
#include <unordered_map>
#include <string>
#include <memory>

class AssetManager {
public:
	static AssetManager& GetInstance() {
		static AssetManager instance;
		return instance;
	}

private:
	AssetManager() {};

	/**
	 * \brief Retrieves the map of assets for a specific type.
	 *
	 * \tparam T The type of the assets.
	 * \return A reference to the map of assets for the specified type.
	 */
	template <typename T>
	std::unordered_map<std::string, std::shared_ptr<T>>& GetAssetMap() {
		// Returns a singleton container for each asset type T.
		static std::unordered_map<std::string, std::shared_ptr<T>> assetMap;
		return assetMap;
	}
};