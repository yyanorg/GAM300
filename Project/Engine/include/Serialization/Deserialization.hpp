#pragma once
// RapidJSON
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

//#include "../config.hpp"

class ECS;
class AssetManager;
class SceneFile;

using AssetID = unsigned;

#if 0

// Handles (de)serialisation of config settings
std::tuple<WindowSettings, GameSettings, GraphicsSettings, UISettings, AudioSettings, LevelEditorSettings>
deserialiseConfig(std::string const& filename, std::string const& audioFilename);

void serialiseConfig(std::string const& audioFileName, float bgmPercentage, float sfxPercentage);

// Handles (de)serialisation of Assets
void deserialiseAssetMetadata(ECS& registry, AssetManager& assetManager, std::string const& filename);

// Handles (de)serialisation of assets folders
std::unordered_map<std::string, std::unordered_map<std::string, std::vector<unsigned int>>>
deserialiseAssetFolders(std::string const& filename);

void preloadAssets(std::string const& name, AssetManager& assetManager);

// Handles (de)serialisation of Scenes
void deserialiseScene(ECS& registry, std::string const& filename, AssetManager& assetManager);

// Handles (de)serialisation of Prefabs
bool deserialisePrefab(ECS& registry, std::string const& key, std::string const& filename, AssetID assetId);

#ifndef DISABLE_IMGUI_LEVELEDITOR
void serialiseAssetFolders(std::unordered_map<std::string, std::unordered_map<std::string, std::vector<unsigned int>>> const assetSubFolders,
    std::string const& filename);
void serialiseAssetMetadata(AssetManager& assetManager, std::string const& filename);
void serialiseScenes(ECS const& registry, AssetManager& assetManager);
void serialiseScene(ECS const& registry, SceneFile const& sceneFile);
std::vector<AssetID> serialisePrefab(ECS& registry, AssetManager& assetManager, AssetID id);
#endif

// Helper functions to (de)serialise Components
namespace deserializer {
    template <typename T>
    T deserializeComponent(rapidjson::Value const& jsonObj);
}

namespace serializer {
    template <typename T>
    std::tuple<std::string, rapidjson::Value, std::vector<AssetID>>
        serializeComponent(T const& component, rapidjson::Document::AllocatorType& alloc);
}

void serialiseConfig(std::string const& filename,
    WindowSettings const& window, GameSettings const& game,
    GraphicsSettings const& graphics, UISettings const& ui,
#ifndef DISABLE_IMGUI_LEVELEDITOR
    LevelEditorSettings const& editor
#else
    LevelEditorSettings const& editor = {}
#endif
);

#endif

#include "Deserialization.ipp"