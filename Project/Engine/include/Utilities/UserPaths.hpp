#pragma once

#include <filesystem>

namespace UserPaths {
    // Empty paths mean the user directory could not be resolved. Callers must
    // skip persistence instead of falling back to the installation directory.
    std::filesystem::path ConfigDirectory();
    std::filesystem::path StateDirectory();
    std::filesystem::path CacheDirectory();
}
