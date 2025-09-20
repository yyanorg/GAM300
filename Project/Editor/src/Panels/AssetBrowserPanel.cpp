#include "Panels/AssetBrowserPanel.hpp"
#include "Asset Manager/MetaFilesManager.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <algorithm>
#include <iostream>
#include <cstring>
#include <cmath>
//#include "Asset Manager/AssetManager.hpp"
#include "Graphics/TextureManager.h"

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

// Thumbnail/grid
static constexpr float THUMBNAILBASESIZE = 96.0f;
static constexpr float THUMBNAILMINSIZE = 48.0f;
static constexpr float THUMBNAILPADDING = 8.0f;
static constexpr float LABELHEIGHT = 18.0f;

AssetBrowserPanel::AssetInfo::AssetInfo(const std::string& path, const GUID_128& g, bool isDir)
    : filePath(path), guid(g), isDirectory(isDir) {
    fileName = std::filesystem::path(path).filename().string();
    extension = std::filesystem::path(path).extension().string();
}

AssetBrowserPanel::AssetBrowserPanel()
    : EditorPanel("Asset Browser", true)
    , currentDirectory("Resources")
    , rootAssetDirectory("Resources")
    , selectedAssetType(AssetType::All)
{
    // Initialize default GUID for untracked assets
    lastSelectedAsset = GUID_128{ 0, 0 };

    // Ensure assets directory exists
    EnsureDirectoryExists(rootAssetDirectory);

    // Initialize file watcher for hot-reloading
    InitializeFileWatcher();

    std::cout << "[AssetBrowserPanel] Initialized with root directory: " << rootAssetDirectory << std::endl;
}

AssetBrowserPanel::~AssetBrowserPanel() {
    // FileWatch destructor will handle cleanup automatically
}

void AssetBrowserPanel::InitializeFileWatcher() {
    try {
        // Create file watcher for the Resources directory
        fileWatcher = std::make_unique<filewatch::FileWatch<std::string>>(
            rootAssetDirectory,
            [this](const std::string& path, const filewatch::Event& event) {
                OnFileChanged(path, event);
            }
        );

        std::cout << "[AssetBrowserPanel] File watcher initialized for: " << rootAssetDirectory << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "[AssetBrowserPanel] Failed to initialize file watcher: " << e.what() << std::endl;
    }
}

void AssetBrowserPanel::OnFileChanged(const std::string& filePath, const filewatch::Event& event) {
    // Process file change on a separate thread-safe context
    ProcessFileChange(filePath, event);
}

void AssetBrowserPanel::ProcessFileChange(const std::string& relativePath, const filewatch::Event& event) {
    // Log the file change for debugging
    const char* eventStr = "";
    switch (event) {
    case filewatch::Event::added: eventStr = "ADDED"; break;
    case filewatch::Event::removed: eventStr = "REMOVED"; break;
    case filewatch::Event::modified: eventStr = "MODIFIED"; break;
    case filewatch::Event::renamed_old: eventStr = "RENAMED_OLD"; break;
    case filewatch::Event::renamed_new: eventStr = "RENAMED_NEW"; break;
    }

    std::cout << "[AssetBrowserPanel] File " << eventStr << ": " << relativePath << std::endl;

    // Build full path from rootAssetDirectory + relativePath
    std::filesystem::path fullPathPath = std::filesystem::path(rootAssetDirectory) / relativePath;
    const std::string fullPath = fullPathPath.generic_string();
    std::filesystem::path fullPathObj(fullPath);
    try {
        if (std::filesystem::exists(fullPathObj) && std::filesystem::is_directory(fullPathObj)) {
            // Directory created/modified � refresh UI
            QueueRefresh();
            return;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[AssetBrowserPanel] Filesystem check error for " << fullPath << ": " << e.what() << std::endl;
    }

    // Only process valid asset files
    std::filesystem::path pathObj(relativePath);
    std::string extension = pathObj.extension().string();
    if (!IsValidAssetFile(extension) && event != filewatch::Event::removed) {
        return;
    }

    // Handle different file events
    switch (event) {
    case filewatch::Event::added:
        if (!MetaFilesManager::MetaFileExists(fullPath)) {
            MetaFilesManager::GenerateMetaFile(fullPath);
        }
        QueueRefresh();
        break;
    case filewatch::Event::renamed_new:
        // For new files, ensure meta file is generated
        if (event == filewatch::Event::added) {
            std::string fullPath = rootAssetDirectory + "/" + relativePath;
            if (!MetaFilesManager::MetaFileExists(fullPath)) {
                MetaFilesManager::GenerateMetaFile(fullPath);
            }
        }
        QueueRefresh();
        break;

    case filewatch::Event::removed:
        QueueRefresh();
        break;
    case filewatch::Event::renamed_old:
        // For removed files, we should clean up meta files
        QueueRefresh();
        break;

    case filewatch::Event::modified:
        // For modified files, update meta file if needed
    {
        std::string fullPath = rootAssetDirectory + "/" + relativePath;
        if (MetaFilesManager::MetaFileExists(fullPath)) {
            MetaFilesManager::UpdateMetaFile(fullPath);
        }
        QueueRefresh();
        break;
    }

    }
}

void AssetBrowserPanel::QueueRefresh() {
    // Set atomic flag to indicate refresh is needed
    refreshPending.store(true);
}

void AssetBrowserPanel::OnImGuiRender() {
    // Check if refresh is needed (from file watcher)
    if (refreshPending.exchange(false)) {
        std::cout << "[AssetBrowserPanel] Refreshing assets due to file changes." << std::endl;
        RefreshAssets();
    }

    if (ImGui::Begin(name.c_str(), &isOpen)) {
        // Render toolbar
        RenderToolbar();
        ImGui::Separator();

        // Create splitter for folder tree and asset grid
        ImGui::BeginChild("##AssetBrowserContent", ImVec2(0, 0), false);

        // Use splitter to divide left panel (folder tree) and right panel (asset grid)
        static float splitterWidth = 250.0f;
        const float MIN_WIDTH = 150.0f;
        const float maxWidth = ImGui::GetContentRegionAvail().x - 200.0f;

        ImGui::BeginChild("##FolderTree", ImVec2(splitterWidth, 0), true);
        RenderFolderTree();
        ImGui::EndChild();

        ImGui::SameLine();

        // Splitter bar
        ImGui::Button("##Splitter", ImVec2(8.0f, -1));
        if (ImGui::IsItemActive()) {
            float delta = ImGui::GetIO().MouseDelta.x;
            splitterWidth += delta;
            splitterWidth = std::clamp(splitterWidth, MIN_WIDTH, maxWidth);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }

        ImGui::SameLine();

        ImGui::BeginChild("##AssetGrid", ImVec2(0, 0), true);
        RenderAssetGrid();
        ImGui::EndChild();

        ImGui::EndChild();
    }
    ImGui::End();
}

void AssetBrowserPanel::RenderToolbar() {
    // Breadcrumb navigation
    ImGui::Text("Path:");
    ImGui::SameLine();

    if (ImGui::SmallButton("Resources")) {
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

    ImGui::SameLine();
    if (ImGui::Button("New Folder")) {
        std::string newFolderPath = currentDirectory + "/New Folder";
        EnsureDirectoryExists(newFolderPath);
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
        RenderDirectoryNode(std::filesystem::path(rootAssetDirectory), "Resources");
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
    }
    catch (const std::exception&) {
        // Ignore errors for inaccessible directories
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (!hasSubdirectories) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    // Highlight if this is the current directory
    if (std::filesystem::path(directory).generic_string() == currentDirectory) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // Use unique ID per node to avoid duplicate-label issues
    std::string nodeId = directory.generic_string();
    ImGui::PushID(nodeId.c_str());
    bool nodeOpen = ImGui::TreeNodeEx(displayName.c_str(), flags);

    // Handle selection
    if (ImGui::IsItemClicked()) {
        NavigateToDirectory(directory.string());
    }

    // Render subdirectories if opened
    if (nodeOpen) {
        if (hasSubdirectories) {
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
            }
            catch (const std::exception&) {
                // Ignore errors for inaccessible directories
            }
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void AssetBrowserPanel::RenderAssetGrid() {
    ImGui::Text("Assets in: %s", GetRelativePath(currentDirectory).c_str());
    ImGui::Separator();

    // Use content region width as the source of truth
    float avail = ImGui::GetContentRegionAvail().x;
    float pad = THUMBNAILPADDING;

    // Compute columns from available width using the base thumbnail size,
    // then compute an even thumbnail size so items fill the row.
    int cols = std::max(1, static_cast<int>(std::floor((avail + pad) / (THUMBNAILBASESIZE + pad))));
    float thumb = (avail - pad * (cols - 1)) / static_cast<float>(cols);
    if (thumb < THUMBNAILMINSIZE) {
        thumb = THUMBNAILMINSIZE;
        cols = std::max(1, static_cast<int>(std::floor((avail + pad) / (thumb + pad))));
        thumb = (avail - pad * (cols - 1)) / static_cast<float>(cols);
    }

    bool anyItemClickedInGrid = false;
    ImGuiIO& io = ImGui::GetIO();

    int index = 0;
    for (const auto& asset : currentAssets) {
        if (!PassesFilter(asset)) continue;

        ImGui::BeginGroup();
        ImGui::PushID(asset.filePath.c_str());

        // Hitbox: thumbnail + label
        ImGui::InvisibleButton("cell", ImVec2(thumb, thumb + LABELHEIGHT));
        bool hovered = ImGui::IsItemHovered();
        bool clicked = ImGui::IsItemClicked();

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 rectMin = ImGui::GetItemRectMin();
        ImVec2 rectMax = ImGui::GetItemRectMax();
        ImVec2 imgMin = rectMin;
        ImVec2 imgMax = ImVec2(rectMin.x + thumb, rectMin.y + thumb);

        // Thumbnail fallback
        ImU32 bg = IM_COL32(80, 80, 80, 255);
        ImU32 border = IM_COL32(100, 100, 100, 255);
        dl->AddRectFilled(imgMin, imgMax, bg, 4.0f);
        dl->AddRect(imgMin, imgMax, border, 4.0f);

        // Centered short name inside thumbnail
        std::string name = asset.fileName;
        const size_t maxChars = 12;
        if (name.size() > maxChars) name = name.substr(0, maxChars - 3) + "...";
        ImVec2 textSize = ImGui::CalcTextSize(name.c_str());
        ImVec2 textPos(imgMin.x + (thumb - textSize.x) * 0.5f, imgMin.y + (thumb - textSize.y) * 0.5f);
        dl->AddText(textPos, IM_COL32(220, 220, 220, 255), name.c_str());

        // Label below thumbnail
        ImGui::SetCursorScreenPos(ImVec2(imgMin.x, imgMax.y));
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + thumb);
        ImGui::TextWrapped("%s", asset.fileName.c_str());
        ImGui::PopTextWrapPos();

        // Selection / activation
        if (clicked) {
            bool ctrl = io.KeyCtrl;
            SelectAsset(asset.guid, ctrl);
            anyItemClickedInGrid = true;
        }

        bool selected = IsAssetSelected(asset.guid);
        if (selected) {
            dl->AddRectFilled(rectMin, rectMax, IM_COL32(100, 150, 255, 50));
            dl->AddRect(rectMin, rectMax, IM_COL32(100, 150, 255, 120), 4.0f, ImDrawFlags_RoundCornersAll, 2.0f);
        }
        else if (hovered) {
            dl->AddRect(rectMin, rectMax, IM_COL32(255, 255, 255, 30), 4.0f, ImDrawFlags_RoundCornersAll, 2.0f);
        }

        if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (asset.isDirectory) NavigateToDirectory(asset.filePath);
            else std::cout << "[AssetBrowserPanel] Opening asset: "
                << "GUID(high=" << asset.guid.high << ", low=" << asset.guid.low << ")"
                << std::endl;
        }

        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            SelectAsset(asset.guid, false);
            ImGui::OpenPopup("AssetContextMenu");
        }

        ImGui::PopID();
        ImGui::EndGroup();

        ++index;
        if ((index % cols) != 0) ImGui::SameLine(0.0f, pad);
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !anyItemClickedInGrid && ImGui::IsWindowHovered()) {
        selectedAssets.clear();
        lastSelectedAsset = GUID_128{ 0, 0 };
    }

    // Context menu
    if (ImGui::BeginPopup("AssetContextMenu")) {
        AssetInfo* contextAsset = nullptr;
        for (auto& a : currentAssets) {
            if (IsAssetSelected(a.guid)) { contextAsset = &a; break; }
        }
        if (contextAsset) ShowAssetContextMenu(*contextAsset);
        ImGui::EndPopup();
    }
}

void AssetBrowserPanel::RefreshAssets() {
    currentAssets.clear();

    try {
        if (!std::filesystem::exists(currentDirectory)) {
            currentDirectory = rootAssetDirectory;
        }

        for (const auto& entry : std::filesystem::directory_iterator(currentDirectory)) {
            // Use normalized generic_string() for map lookups and storage
            std::string filePath = entry.path().generic_string();
            bool isDirectory = entry.is_directory();

            GUID_128 guid{ 0, 0 };

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

                // Get or generate GUID using normalized filePath
                if (MetaFilesManager::MetaFileExists(filePath)) {
                    guid = MetaFilesManager::GetGUID128FromAssetFile(filePath);
                }
                else {
                    guid = MetaFilesManager::GenerateMetaFile(filePath);
                }
            }
            else {
                // For directories, generate a simple hash-based GUID using normalized path
                std::hash<std::string> hasher;
                size_t hash = hasher(filePath);
                guid.high = static_cast<uint64_t>(hash);
                guid.low = static_cast<uint64_t>(hash >> 32);
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

    }
    catch (const std::exception& e) {
        std::cerr << "[AssetBrowserPanel] Error refreshing assets: " << e.what() << std::endl;
    }

    UpdateBreadcrumbs();
}

void AssetBrowserPanel::NavigateToDirectory(const std::string& directory) {
    std::string normalizedPath = std::filesystem::path(directory).generic_string();

    if (std::filesystem::exists(normalizedPath) && std::filesystem::is_directory(normalizedPath)) {
        currentDirectory = normalizedPath;
        selectedAssets.clear();
        lastSelectedAsset = GUID_128{ 0, 0 };
        RefreshAssets(); // Immediate refresh when navigating
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
    }
    else if (lowerExt == ".obj" || lowerExt == ".fbx" || lowerExt == ".dae" || lowerExt == ".3ds") {
        return AssetType::Models;
    }
    else if (lowerExt == ".vert" || lowerExt == ".frag" || lowerExt == ".glsl" || lowerExt == ".hlsl") {
        return AssetType::Shaders;
    }
    else if (lowerExt == ".wav" || lowerExt == ".mp3" || lowerExt == ".ogg") {
        return AssetType::Audio;
    }
    else if (lowerExt == ".ttf" || lowerExt == ".otf") {
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
    }
    else {
        selectedAssets.insert(guid);
        lastSelectedAsset = guid;
    }
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
        }
        else {
            std::filesystem::remove(asset.filePath);
            // Also remove meta file
            std::string metaFile = asset.filePath + ".meta";
            if (std::filesystem::exists(metaFile)) {
                std::filesystem::remove(metaFile);
            }
        }
    }
    catch (const std::exception& e) {
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

std::string AssetBrowserPanel::GetRelativePath(const std::string& fullPath) const {
    try {
        std::filesystem::path relative = std::filesystem::relative(fullPath, rootAssetDirectory);
        return relative.generic_string();
    }
    catch (const std::exception&) {
        return fullPath;
    }
}

bool AssetBrowserPanel::IsValidAssetFile(const std::string& extension) const {
    std::string lowerExt = extension;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);

    static const std::unordered_set<std::string> VALID_EXTENSIONS = {
        ".png", ".jpg", ".jpeg", ".bmp", ".tga",           // Textures
        ".obj", ".fbx", ".dae", ".3ds",                    // Models
        ".vert", ".frag", ".glsl", ".hlsl",                // Shaders
        ".wav", ".mp3", ".ogg",                            // Audio
        ".ttf", ".otf"                                     // Fonts
    };

    return VALID_EXTENSIONS.count(lowerExt) > 0;
}

void AssetBrowserPanel::EnsureDirectoryExists(const std::string& directory) {
    try {
        if (!std::filesystem::exists(directory)) {
            std::filesystem::create_directories(directory);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[AssetBrowserPanel] Failed to create directory " << directory << ": " << e.what() << std::endl;
    }
}