#pragma once
#include <string>
#include "Asset Manager/GUID.hpp"
#include "Asset Manager/AssetMeta.hpp"

class IAsset {
public:
	virtual ~IAsset() = default;
	virtual std::string CompileToResource(const std::string& assetPath) = 0;
	virtual bool LoadResource(const std::string& assetPath) { return true; }
	std::shared_ptr<AssetMeta> GenerateBaseMetaFile(GUID_128 guid128, const std::string& assetPath, const std::string& resourcePath);
	virtual std::shared_ptr<AssetMeta> ExtendMetaFile(const std::string& assetPath, std::shared_ptr<AssetMeta> currentMetaData) = 0;
};