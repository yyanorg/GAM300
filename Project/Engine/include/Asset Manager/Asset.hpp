#pragma once
#include <string>

class IAsset {
public:
	virtual ~IAsset() = default;
	virtual bool CompileToResource(const std::string& path) = 0;
	virtual bool LoadResource(const std::string& path) = 0;
};