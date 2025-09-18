#pragma once
#include "Asset Manager/GUID.hpp"
#include <string>
#include <chrono>

class AssetMeta {
public:
	GUID_128 guid{};
	std::string sourceFilePath;
	std::string compiledFilePath;
	std::chrono::system_clock::time_point lastCompileTime;
	int version{};
};

class TextureMeta : public AssetMeta {
public:
	uint32_t ID{};
	std::string type;
	uint32_t unit{};
};