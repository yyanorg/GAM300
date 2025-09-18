#include "Panels/AssetBrowserPanel.hpp"
#include "Asset Manager/MetaFilesManager.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <algorithm>
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

AssetBrowserPanel::AssetInfo::AssetInfo(const std::string& path, const GUID_128& g, bool isDir) 
    : filePath(path), guid(g), isDirectory(isDir) {
    fileName = std::filesystem::path(path).filename().string();
    extension = std::filesystem::path(path).extension().string();
    try {
        lastWriteTime = std::filesystem::last_write_time(path);
    } catch (...) {
        lastWriteTime = std::filesystem::file_time_type{};
    }
}

AssetBrowserPanel::AssetBrowserPanel() 
    : EditorPanel("Asset Browser", true)
    , currentDirectory("Resources")
    , rootAssetDirectory("Resources")
    , selectedAssetType(AssetType::All)
    , isRefreshPending(true)
{
    // Initialize default GUID for untracked assets
    lastSelectedAsset = GUID_128{0, 0};
    
    // Ensure assets directory exists
    EnsureDirectoryExists(rootAssetDirectory);
    
    std::cout << "[AssetBrowserPanel] Initialized with root directory: " << rootAssetDirectory << std::endl;
}

AssetBrowserPanel::~AssetBrowserPanel() {
    // Nothing to cleanup for now
}

void AssetBrowserPanel::OnImGuiRender() {
    if (ImGui::Begin(name.c_str(), &isOpen)) {
        // Check if refresh is needed
        if (isRefreshPending) {
            RefreshAssets();
            isRefreshPending = false;
        }
        
        // Render toolbar
        RenderToolbar();
        ImGui::Separator();
        
        // Create splitter for folder tree and asset grid
        ImGui::BeginChild("##AssetBrowserContent", ImVec2(0, 0), false);
        
        // Use splitter to divide left panel (folder tree) and right panel (asset grid)
        static float splitter_width = 250.0f;
        const float min_width = 150.0f;
        const float max_width = ImGui::GetContentRegionAvail().x - 200.0f;
        
        ImGui::BeginChild("##FolderTree", ImVec2(splitter_width, 0), true);
        RenderFolderTree();
        ImGui::EndChild();
        
        ImGui::SameLine();
        
        // Splitter bar
        ImGui::Button("##Splitter", ImVec2(8.0f, -1));
        if (ImGui::IsItemActive()) {
            float delta = ImGui::GetIO().MouseDelta.x;
            splitter_width += delta;
            splitter_width = std::clamp(splitter_width, min_width, max_width);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }
        
        ImGui::SameLine();
        
        ImGui::BeginChild("##AssetGrid", ImVec2(0, 0), true);
        RenderAssetGrid();
        ImGui::EndChild();
        
        ImGui::EndChild();
        
        // Status bar
        RenderStatusBar();
    }
    ImGui::End();
}

void AssetBrowserPanel::RenderToolbar() {
    // Breadcrumb navigation
    ImGui::Text("Path:");
    ImGui::SameLine();
    
    if (ImGui::SmallButton("Assets")) {
        NavigateToDirectory(rootAssetDirectory);
    }
    
    for (size_t i = 0; i < pathBreadcrumbs.size(); ++i) {
        ImGui::SameLine();
        ImGui::Text("/");
        ImGui::SameLine();
        
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::SmallButton(pathBreadcrumbs[i].c_str())) {
            std::string targetPath = rootAssetDirectory;
            for (size_t j = 0; j <= i; ++j) {
                targetPath += "/" + pathBreadcrumbs[j];
            }
            NavigateToDirectory(targetPath);
        }
        ImGui::PopID();
    }
    
    // Toolbar buttons
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 200.0f);
    
    if (ImGui::Button("Refresh")) {
        isRefreshPending = true;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("New Folder")) {
        std::string newFolderPath = currentDirectory + "/New Folder";
        EnsureDirectoryExists(newFolderPath);
        isRefreshPending = true;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Import")) {
        // TODO: Implement import dialog
    }
    
    // Search and filter bar
    ImGui::SetNextItemWidth(200.0f);
    char searchBuffer[256];
    strncpy_s(searchBuffer, searchQuery.c_str(), sizeof(searchBuffer) - 1);
    searchBuffer[sizeof(searchBuffer) - 1] = '\0';
    
    if (ImGui::InputTextWithHint("##Search", "Search assets...", searchBuffer, sizeof(searchBuffer))) {
        searchQuery = searchBuffer;
    }
    
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    
    const char* assetTypeNames[] = { "All", "Textures", "Models", "Shaders", "Audio", "Fonts" };
    int currentTypeIndex = static_cast<int>(selectedAssetType);
    if (ImGui::Combo("##Filter", &currentTypeIndex, assetTypeNames, IM_ARRAYSIZE(assetTypeNames))) {
        selectedAssetType = static_cast<AssetType>(currentTypeIndex);
    }
}

void AssetBrowserPanel::RenderFolderTree() {
    ImGui::Text("Folders");
    ImGui::Separator();
    
    // Render root directory
    if (std::filesystem::exists(rootAssetDirectory)) {
        RenderDirectoryNode(std::filesystem::path(rootAssetDirectory), "Assets");
    }
}

void AssetBrowserPanel::RenderDirectoryNode(const std::filesystem::path& directory, const std::string& displayName) {
    bool hasSubdirectories = false;
    
    // Check if this directory has subdirectories
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_directory()) {
                hasSubdirectories = true;
                break;
            }
        }
    } catch (const std::exception&) {
        // Ignore errors for inaccessible directories
    }
    
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (!hasSubdirectories) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    
    // Highlight if this is the current directory
    if (directory.string() == currentDirectory) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    
    bool nodeOpen = ImGui::TreeNodeEx(displayName.c_str(), flags);
    
    // Handle selection
    if (ImGui::IsItemClicked()) {
        NavigateToDirectory(directory.string());
    }
    
    // Render subdirectories
    if (nodeOpen && hasSubdirectories) {
        try {
            std::vector<std::filesystem::path> subdirectories;
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                if (entry.is_directory()) {
                    subdirectories.push_back(entry.path());
                }
            }
            
            // Sort subdirectories
            std::sort(subdirectories.begin(), subdirectories.end());
            
            for (const auto& subdir : subdirectories) {
                RenderDirectoryNode(subdir, subdir.filename().string());
            }
        } catch (const std::exception&) {
            // Ignore errors for inaccessible directories
        }
        
        ImGui::TreePop();
    }
}

void AssetBrowserPanel::RenderAssetGrid() {
    // Header with current directory name
    ImGui::Text("Assets in: %s", GetRelativePath(currentDirectory).c_str());
    ImGui::Separator();
    
    // Show assets in a simple list format
    for (const auto& asset : currentAssets) {
        if (PassesFilter(asset)) {
            bool isSelected = IsAssetSelected(asset.guid);
            
            // Show folder or file icon
            if (asset.isDirectory) {
                ImGui::Text("[DIR]  %s", asset.fileName.c_str());
            } else {
                std::string typeStr;
                AssetType type = GetAssetTypeFromExtension(asset.extension);
                switch (type) {
                    case AssetType::Textures: typeStr = "[IMG]"; break;
                    case AssetType::Models: typeStr = "[MDL]"; break;
                    case AssetType::Shaders: typeStr = "[SHD]"; break;
                    case AssetType::Audio: typeStr = "[AUD]"; break;
                    case AssetType::Fonts: typeStr = "[FNT]"; break;
                    default: typeStr = "[FILE]"; break;
                }
                ImGui::Text("%s  %s", typeStr.c_str(), asset.fileName.c_str());
            }
            
            // Make the whole line selectable
            if (ImGui::IsItemClicked()) {
                bool multiSelect = ImGui::GetIO().KeyCtrl;
                SelectAsset(asset.guid, multiSelect);
            }
            
            // Highlight selected items
            if (isSelected) {
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 rectMin = ImGui::GetItemRectMin();
                ImVec2 rectMax = ImGui::GetItemRectMax();
                drawList->AddRectFilled(rectMin, rectMax, IM_COL32(100, 150, 255, 50));
            }
            
            // Double-click to navigate/open
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (asset.isDirectory) {
                    NavigateToDirectory(asset.filePath);
                } else {
                    std::cout << "[AssetBrowserPanel] Opening asset: " << asset.fileName << std::endl;
                }
            }
            
            // Context menu
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                SelectAsset(asset.guid, false);
                ImGui::OpenPopup("AssetContextMenu");
            }
            
            //// Drag and drop
            //if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            //    HandleDragAndDrop(asset);
            //    ImGui::EndDragDropSource();
            //}
        }
    }
    
    // Context menu popup
    if (ImGui::BeginPopup("AssetContextMenu")) {
        // Find the selected asset for context menu
        AssetInfo* contextAsset = nullptr;
        for (auto& asset : currentAssets) {
            if (IsAssetSelected(asset.guid)) {
                contextAsset = &asset;
                break;
            }
        }
        
        if (contextAsset) {
            ShowAssetContextMenu(*contextAsset);
        }
        ImGui::EndPopup();
    }
}

void AssetBrowserPanel::RenderStatusBar() {
    ImGui::Separator();
    ImGui::Text("Assets: %zu | Selected: %zu", currentAssets.size(), selectedAssets.size());
}

void AssetBrowserPanel::RefreshAssets() {
    currentAssets.clear();
    
    try {
        if (!std::filesystem::exists(currentDirectory)) {
            currentDirectory = rootAssetDirectory;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(currentDirectory)) {
            std::string filePath = entry.path().string();
            bool isDirectory = entry.is_directory();
            
            GUID_128 guid{0, 0};
            
            if (!isDirectory) {
                // Skip meta files
                if (entry.path().extension() == ".meta") {
                    continue;
                }
                
                // Check if it's a valid asset file
                std::string extension = entry.path().extension().string();
                if (!IsValidAssetFile(extension)) {
                    continue;
                }
                
                // Get or generate GUID
                if (MetaFilesManager::MetaFileExists(filePath)) {
                    guid = MetaFilesManager::GetGUID128FromAssetFile(filePath);
                } else {
                    guid = MetaFilesManager::GenerateMetaFile(filePath);
                }
            } else {
                // For directories, generate a simple hash-based GUID
                std::hash<std::string> hasher;
                size_t hash = hasher(filePath);
                guid.high = hash;
                guid.low = hash >> 32;
            }
            
            currentAssets.emplace_back(filePath, guid, isDirectory);
        }
        
        // Sort assets: directories first, then files
        std::sort(currentAssets.begin(), currentAssets.end(), [](const AssetInfo& a, const AssetInfo& b) {
            if (a.isDirectory != b.isDirectory) {
                return a.isDirectory > b.isDirectory;
            }
            return a.fileName < b.fileName;
        });
        
    } catch (const std::exception& e) {
        std::cerr << "[AssetBrowserPanel] Error refreshing assets: " << e.what() << std::endl;
    }
    
    UpdateBreadcrumbs();
}

void AssetBrowserPanel::NavigateToDirectory(const std::string& directory) {
    std::string normalizedPath = std::filesystem::path(directory).generic_string();
    
    if (std::filesystem::exists(normalizedPath) && std::filesystem::is_directory(normalizedPath)) {
        currentDirectory = normalizedPath;
        ClearSelection();
        isRefreshPending = true;
    }
}

void AssetBrowserPanel::UpdateBreadcrumbs() {
    pathBreadcrumbs.clear();
    
    std::filesystem::path relativePath = std::filesystem::relative(currentDirectory, rootAssetDirectory);
    
    if (relativePath != ".") {
        for (const auto& part : relativePath) {
            pathBreadcrumbs.push_back(part.string());
        }
    }
}

bool AssetBrowserPanel::PassesFilter(const AssetInfo& asset) const {
    // Search filter
    if (!searchQuery.empty()) {
        std::string lowerFileName = asset.fileName;
        std::string lowerSearch = searchQuery;
        std::transform(lowerFileName.begin(), lowerFileName.end(), lowerFileName.begin(), ::tolower);
        std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);
        
        if (lowerFileName.find(lowerSearch) == std::string::npos) {
            return false;
        }
    }
    
    // Type filter
    if (selectedAssetType != AssetType::All && !asset.isDirectory) {
        AssetType assetType = GetAssetTypeFromExtension(asset.extension);
        if (assetType != selectedAssetType) {
            return false;
        }
    }
    
    return true;
}

AssetBrowserPanel::AssetType AssetBrowserPanel::GetAssetTypeFromExtension(const std::string& extension) const {
    std::string lowerExt = extension;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
    
    if (lowerExt == ".png" || lowerExt == ".jpg" || lowerExt == ".jpeg" || lowerExt == ".bmp" || lowerExt == ".tga") {
        return AssetType::Textures;
    } else if (lowerExt == ".obj" || lowerExt == ".fbx" || lowerExt == ".dae" || lowerExt == ".3ds") {
        return AssetType::Models;
    } else if (lowerExt == ".vert" || lowerExt == ".frag" || lowerExt == ".glsl" || lowerExt == ".hlsl") {
        return AssetType::Shaders;
    } else if (lowerExt == ".wav" || lowerExt == ".mp3" || lowerExt == ".ogg") {
        return AssetType::Audio;
    } else if (lowerExt == ".ttf" || lowerExt == ".otf") {
        return AssetType::Fonts;
    }
    
    return AssetType::All;
}

void AssetBrowserPanel::SelectAsset(const GUID_128& guid, bool multiSelect) {
    if (!multiSelect) {
        selectedAssets.clear();
    }
    
    if (selectedAssets.count(guid)) {
        selectedAssets.erase(guid);
    } else {
        selectedAssets.insert(guid);
        lastSelectedAsset = guid;
    }
}

void AssetBrowserPanel::ClearSelection() {
    selectedAssets.clear();
    lastSelectedAsset = GUID_128{0, 0};
}

bool AssetBrowserPanel::IsAssetSelected(const GUID_128& guid) const {
    return selectedAssets.count(guid) > 0;
}

void AssetBrowserPanel::ShowAssetContextMenu(const AssetInfo& asset) {
    if (ImGui::MenuItem("Open")) {
        std::cout << "[AssetBrowserPanel] Opening: " << asset.fileName << std::endl;
    }
    
    ImGui::Separator();
    
    if (ImGui::MenuItem("Reveal in Explorer")) {
        RevealInExplorer(asset);
    }
    
    if (ImGui::MenuItem("Copy Path")) {
        CopyAssetPath(asset);
    }
    
    if (!asset.isDirectory && ImGui::MenuItem("Show Meta File")) {
        ShowMetaFile(asset);
    }
    
    ImGui::Separator();
    
    if (ImGui::MenuItem("Delete", nullptr, false, !asset.isDirectory)) {
        DeleteAsset(asset);
    }
}

void AssetBrowserPanel::HandleDragAndDrop(const AssetInfo& asset) {
    // Set drag and drop payload with GUID
    ImGui::SetDragDropPayload("ASSET_GUID", &asset.guid, sizeof(GUID_128));
    
    // Show preview
    ImGui::Text("Dragging: %s", asset.fileName.c_str());
}

void AssetBrowserPanel::DeleteAsset(const AssetInfo& asset) {
    try {
        if (asset.isDirectory) {
            std::filesystem::remove_all(asset.filePath);
        } else {
            std::filesystem::remove(asset.filePath);
            // Also remove meta file
            std::string metaFile = asset.filePath + ".meta";
            if (std::filesystem::exists(metaFile)) {
                std::filesystem::remove(metaFile);
            }
        }
        isRefreshPending = true;
    } catch (const std::exception& e) {
        std::cerr << "[AssetBrowserPanel] Failed to delete asset: " << e.what() << std::endl;
    }
}

void AssetBrowserPanel::RevealInExplorer(const AssetInfo& asset) {
#ifdef _WIN32
    std::string command = "explorer /select,\"" + asset.filePath + "\"";
    system(command.c_str());
#else
    std::cout << "[AssetBrowserPanel] Reveal in explorer not implemented for this platform" << std::endl;
#endif
}

void AssetBrowserPanel::CopyAssetPath(const AssetInfo& asset) {
    std::string relativePath = GetRelativePath(asset.filePath);
    
#ifdef _WIN32
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, relativePath.size() + 1);
        if (hMem) {
            memcpy(GlobalLock(hMem), relativePath.c_str(), relativePath.size() + 1);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
        }
        CloseClipboard();
    }
#else
    std::cout << "[AssetBrowserPanel] Copy to clipboard: " << relativePath << std::endl;
#endif
}

void AssetBrowserPanel::ShowMetaFile(const AssetInfo& asset) {
    std::string metaFile = asset.filePath + ".meta";
    if (std::filesystem::exists(metaFile)) {
        RevealInExplorer(AssetInfo(metaFile, GUID_128{0, 0}, false));
    }
}

std::string AssetBrowserPanel::GetRelativePath(const std::string& fullPath) const {
    try {
        std::filesystem::path relative = std::filesystem::relative(fullPath, rootAssetDirectory);
        return relative.generic_string();
    } catch (const std::exception&) {
        return fullPath;
    }
}

bool AssetBrowserPanel::IsValidAssetFile(const std::string& extension) const {
    std::string lowerExt = extension;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
    
    static const std::unordered_set<std::string> validExtensions = {
        ".png", ".jpg", ".jpeg", ".bmp", ".tga",           // Textures
        ".obj", ".fbx", ".dae", ".3ds",                    // Models
        ".vert", ".frag", ".glsl", ".hlsl",                // Shaders
        ".wav", ".mp3", ".ogg",                            // Audio
        ".ttf", ".otf"                                     // Fonts
    };
    
    return validExtensions.count(lowerExt) > 0;
}

void AssetBrowserPanel::EnsureDirectoryExists(const std::string& directory) {
    try {
        if (!std::filesystem::exists(directory)) {
            std::filesystem::create_directories(directory);
        }
    } catch (const std::exception& e) {
        std::cerr << "[AssetBrowserPanel] Failed to create directory " << directory << ": " << e.what() << std::endl;
    }
}