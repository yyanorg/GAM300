#include "Utilities/UserPaths.hpp"

#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <memory>
#elif defined(ANDROID)
#include "WindowManager.hpp"
#include "Platform/IPlatform.h"
#endif

namespace {
#ifdef _WIN32
    std::filesystem::path LocalDirectory() {
        PWSTR raw = nullptr;
        const HRESULT result = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &raw);
        const std::unique_ptr<wchar_t, decltype(&CoTaskMemFree)> folder(raw, CoTaskMemFree);
        if (FAILED(result) || !folder) return {};
        return std::filesystem::path(folder.get()) / "DigiPen" / "Kusane";
    }
#elif defined(ANDROID)
    std::filesystem::path LocalDirectory() {
        auto* platform = WindowManager::GetPlatform();
        return platform ? std::filesystem::path(platform->GetWritablePath()) : std::filesystem::path{};
    }
#else
    std::filesystem::path XdgDirectory(const char* variable, const char* fallback) {
        if (const char* value = std::getenv(variable); value && *value) {
            const std::filesystem::path directory(value);
            if (directory.is_absolute()) return directory / "Kusane";
        }
        if (const char* value = std::getenv("HOME"); value && *value) {
            const std::filesystem::path directory(value);
            if (directory.is_absolute()) return directory / fallback / "Kusane";
        }
        return {};
    }
#endif
}

std::filesystem::path UserPaths::ConfigDirectory() {
#if defined(_WIN32) || defined(ANDROID)
    return LocalDirectory();
#else
    return XdgDirectory("XDG_CONFIG_HOME", ".config");
#endif
}

std::filesystem::path UserPaths::StateDirectory() {
#if defined(_WIN32) || defined(ANDROID)
    return LocalDirectory();
#else
    return XdgDirectory("XDG_STATE_HOME", ".local/state");
#endif
}

std::filesystem::path UserPaths::CacheDirectory() {
#if defined(_WIN32) || defined(ANDROID)
    auto directory = LocalDirectory();
    return directory.empty() ? directory : directory / "Cache";
#else
    return XdgDirectory("XDG_CACHE_HOME", ".cache");
#endif
}
