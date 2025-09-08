#pragma once
#include <string>

class IAsset {
public:
	virtual ~IAsset() = default;
	virtual bool LoadAsset(const std::string& path) = 0;
};