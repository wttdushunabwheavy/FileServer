#include "FileServer.h"
#include "PathUtils.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace fs = std::filesystem;

namespace
{
    std::string GetRequestTime()
    {
        const auto Now =
            std::chrono::system_clock::now();

        const std::time_t Time =
            std::chrono::system_clock::to_time_t(Now);

        std::tm LocalTime{};

#ifdef _WIN32
        localtime_s(
            &LocalTime,
            &Time
        );
#else
        localtime_r(
            &Time,
            &LocalTime
        );
#endif

        std::ostringstream Stream;

        Stream
            << std::setfill('0')
            << std::setw(2)
            << LocalTime.tm_hour
            << ':'
            << std::setw(2)
            << LocalTime.tm_min;

        return Stream.str();
    }
}

FileServer::FileServer(const ServerConfig& config)
    : Config(config)
    , Root(fs::weakly_canonical(config.FilesDirectory))
{
    RegisterRoutes();
}

void FileServer::RegisterRoutes()
{
    Server.Get(
        "/health",
        [this](
            const httplib::Request& request,
            httplib::Response& response)
        {
            HandleHealth(request, response);
        }
    );

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

    Server.Get(
        R"(/file/(.+))",
        [this](
            const httplib::Request& request,
            httplib::Response& response)
        {
            HandleFile(request, response);
        }
    );

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
    // Time when the file request reached the server.
    // Format: HH:MM, for example 19:31.
    const std::string RequestTime =
        GetRequestTime();

    const std::string requestedPath =
        request.matches[1].str();

    fs::path filePath;

    if (!PathUtils::ResolveSafePath(
            Root,
            requestedPath,
            filePath))
    {
        std::cout
            << '['
            << RequestTime
            << "] [FORBIDDEN] "
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
        << '['
        << RequestTime
        << "] [DOWNLOAD] "
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

    const std::string extension =
        path.extension().string();

    const auto it =
        MimeTypes.find(extension);

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
