#pragma once

#include <filesystem>
#include <string>

namespace PathUtils
{
    bool ResolveSafePath(
        const std::filesystem::path& root,
        const std::string& requestedPath,
        std::filesystem::path& result
    );
}