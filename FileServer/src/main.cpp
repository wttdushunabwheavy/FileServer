#include "Config.h"
#include "FileServer.h"

#include <filesystem>
#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        namespace fs = std::filesystem;

        /*
            Получаем директорию, в которой находится
            сам FileServer.exe.
        */

        const fs::path ServerDirectory =
            fs::absolute(argv[0]).parent_path();

        ServerConfig config;

        config.Host = "0.0.0.0";
        config.Port = 8080;

        /*
            Все пути считаются относительно
            директории FileServer.exe.

            Структура:

            FileServer.exe
            CfgManifest.txt
            Manifest.txt
            files/
        */

        config.FilesDirectory =
            ServerDirectory / "files";

        config.CfgManifest =
            ServerDirectory / "CfgManifest.txt";

        config.Manifest =
            ServerDirectory / "Manifest.txt";

        /*
            Создаём files/, если её ещё нет.
        */

        fs::create_directories(
            config.FilesDirectory
        );

        FileServer server(config);

        if (!server.Start())
        {
            std::cerr
                << "Failed to start server.\n";

            return 1;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "Fatal error: "
            << e.what()
            << '\n';

        return 1;
    }

    return 0;
}