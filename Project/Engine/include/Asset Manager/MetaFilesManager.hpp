#pragma once
#include <string>
#include "GUID.hpp"

class MetaFilesManager {
public:
	//static GUID_128 GenerateMetaFile(const std::string& assetPath, const std::string& resourcePath);

	static bool MetaFileExists(const std::string& assetPath);

	static GUID_string GetGUIDFromMetaFile(const std::string& metaFilePath);

	static GUID_string GetGUIDFromAssetFile(const std::string& assetPath);

	static void InitializeAssetMetaFiles(const std::string& rootAssetFolder);

    static GUID_128 GetGUID128FromAssetFile(const std::string& assetPath);

	static bool MetaFileUpdated(const std::string& assetPath);

	static GUID_128 UpdateMetaFile(const std::string& assetPath);

	static void AddGUID128Mapping(const std::string& assetPath, const GUID_128& guid);

    static constexpr int CURRENT_METADATA_VERSION = 2;

private:
    /**
    * \brief Deleted default constructor to prevent instantiation.
    */
    MetaFilesManager() = delete;

    /**
     * \brief Deleted destructor to prevent instantiation.
     */
    ~MetaFilesManager() = delete;

    static std::unordered_map<std::string, GUID_128> assetPathToGUID128; // Map from an asset's file path to its GUID_128_t value.

};