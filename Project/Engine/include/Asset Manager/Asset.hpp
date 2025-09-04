#pragma once
#include <string>

class IAsset {
public:
	virtual ~IAsset() = default;
	virtual bool LoadFromFile(const std::string& path) = 0;
};