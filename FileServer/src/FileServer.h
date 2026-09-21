#pragma once

#include "Config.h"

#include <filesystem>
#include <httplib.h>

class FileServer
{
public:
    explicit FileServer(const ServerConfig& config);

    bool Start();

private:
    void RegisterRoutes();
    void HandleFile(
        const httplib::Request& request,
        httplib::Response& response
    );

    void HandleHealth(
        const httplib::Request& request,
        httplib::Response& response
    );

    static std::string GetContentType(
        const std::filesystem::path& path
    );

private:
    ServerConfig Config;
    std::filesystem::path Root;

    httplib::Server Server;
};