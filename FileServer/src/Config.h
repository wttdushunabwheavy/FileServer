#pragma once

#include <filesystem>
#include <string>

struct ServerConfig
{
    std::string Host = "0.0.0.0";
    int Port = 8080;

    std::filesystem::path FilesDirectory = "files";

    std::filesystem::path CfgManifest = "CfgManifest.txt";
    std::filesystem::path Manifest = "Manifest.txt";
};
