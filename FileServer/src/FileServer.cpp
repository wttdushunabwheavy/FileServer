#include "FileServer.h"
#include "PathUtils.h"

#include <iostream>
#include <unordered_map>

namespace fs = std::filesystem;

FileServer::FileServer(const ServerConfig& config)
    : Config(config)
    , Root(fs::weakly_canonical(config.FilesDirectory))
{
    RegisterRoutes();
}

void FileServer::RegisterRoutes()
{
    // ------------------------------------------------------------
    // Health
    // ------------------------------------------------------------

    Server.Get(
        "/health",
        [this](
            const httplib::Request& request,
            httplib::Response& response)
        {
            HandleHealth(request, response);
        }
    );

    // ------------------------------------------------------------
    // CfgManifest.txt
    //
    // GET /manifest/cfg
    // ------------------------------------------------------------

    Server.Get(
        "/manifest/cfg",
        [this](
            const httplib::Request&,
            httplib::Response& response)
        {
            const fs::path path = Config.CfgManifest;

            std::error_code ec;

            if (!fs::is_regular_file(path, ec))
            {
                response.status = 404;

                response.set_content(
                    "CfgManifest.txt not found\n",
                    "text/plain"
                );

                return;
            }

            response.set_file_content(
                path.string(),
                "text/plain"
            );
        }
    );

    // ------------------------------------------------------------
    // Manifest.txt
    //
    // GET /manifest
    // ------------------------------------------------------------

    Server.Get(
        "/manifest",
        [this](
            const httplib::Request&,
            httplib::Response& response)
        {
            const fs::path path = Config.Manifest;

            std::error_code ec;

            if (!fs::is_regular_file(path, ec))
            {
                response.status = 404;

                response.set_content(
                    "Manifest.txt not found\n",
                    "text/plain"
                );

                return;
            }

            response.set_file_content(
                path.string(),
                "text/plain"
            );
        }
    );

    // ------------------------------------------------------------
    // Files
    //
    // GET /file/Client.exe
    // GET /file/Game.pak
    // GET /file/Game/Content/foo.pak
    // ------------------------------------------------------------

    Server.Get(
        R"(/file/(.+))",
        [this](
            const httplib::Request& request,
            httplib::Response& response)
        {
            HandleFile(request, response);
        }
    );

    // ------------------------------------------------------------
    // 404
    // ------------------------------------------------------------

    Server.set_error_handler(
        [](
            const httplib::Request&,
            httplib::Response& response)
        {
            if (response.status == 404)
            {
                response.set_content(
                    "Not found\n",
                    "text/plain"
                );
            }
        }
    );
}

void FileServer::HandleHealth(
    const httplib::Request& request,
    httplib::Response& response)
{
    std::cout
        << "[HEALTH] "
        << request.remote_addr
        << '\n';

    response.status = 200;

    response.set_content(
        "OK\n",
        "text/plain"
    );
}

void FileServer::HandleFile(
    const httplib::Request& request,
    httplib::Response& response)
{
    const std::string requestedPath =
        request.matches[1].str();

    fs::path filePath;

    // ------------------------------------------------------------
    // Security:
    // requestedPath must stay inside files/
    // ------------------------------------------------------------

    if (!PathUtils::ResolveSafePath(
            Root,
            requestedPath,
            filePath))
    {
        std::cout
            << "[FORBIDDEN] "
            << request.remote_addr
            << " -> "
            << requestedPath
            << '\n';

        response.status = 403;

        response.set_content(
            "Forbidden\n",
            "text/plain"
        );

        return;
    }

    // ------------------------------------------------------------
    // Check file
    // ------------------------------------------------------------

    std::error_code ec;

    if (!fs::exists(filePath, ec))
    {
        response.status = 404;

        response.set_content(
            "File not found\n",
            "text/plain"
        );

        return;
    }

    if (!fs::is_regular_file(filePath, ec))
    {
        response.status = 404;

        response.set_content(
            "Not a file\n",
            "text/plain"
        );

        return;
    }

    // ------------------------------------------------------------
    // Send file
    //
    // cpp-httplib handles the file-backed response.
    // Range requests can therefore be used by the updater
    // for resumable downloads.
    // ------------------------------------------------------------

    response.set_file_content(
        filePath.string(),
        GetContentType(filePath)
    );

    response.set_header(
        "Content-Disposition",
        "attachment; filename=\"" +
        filePath.filename().string() +
        "\""
    );

    std::cout
        << "[DOWNLOAD] "
        << request.remote_addr
        << " -> "
        << requestedPath
        << '\n';
}

std::string FileServer::GetContentType(
    const fs::path& path)
{
    static const std::unordered_map<
        std::string,
        std::string
    > MimeTypes =
    {
        { ".txt",  "text/plain" },
        { ".json", "application/json" },
        { ".xml",  "application/xml" },

        { ".exe",  "application/octet-stream" },
        { ".dll",  "application/octet-stream" },
        { ".pak",  "application/octet-stream" },
        { ".bin",  "application/octet-stream" },

        { ".zip",  "application/zip" },

        { ".jpg",  "image/jpeg" },
        { ".jpeg", "image/jpeg" },
        { ".png",  "image/png" },

        { ".html", "text/html" },
        { ".css",  "text/css" },
        { ".js",   "application/javascript" }
    };

    std::string extension =
        path.extension().string();

    auto it = MimeTypes.find(extension);

    if (it != MimeTypes.end())
        return it->second;

    return "application/octet-stream";
}

bool FileServer::Start()
{
    std::cout
        << "=================================\n"
        << "       Update File Server\n"
        << "=================================\n"
        << "Root: "
        << Root.string()
        << '\n'
        << "Address: "
        << Config.Host
        << ':'
        << Config.Port
        << "\n\n";

    return Server.listen(
        Config.Host,
        Config.Port
    );
}