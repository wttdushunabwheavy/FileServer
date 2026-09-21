#include "PathUtils.h"

namespace fs = std::filesystem;

namespace PathUtils
{

    bool ResolveSafePath(
        const fs::path& root,
        const std::string& requestedPath,
        fs::path& result
    )
    {
        if (requestedPath.empty())
            return false;

        /*
            URL path должен быть относительным.
    
            Например:
    
                Game/foo.pak
    
            допустимо.
    
                ../../Windows/System32/foo.dll
    
            недопустимо.
        */

        fs::path relative(requestedPath);

        if (relative.is_absolute())
            return false;

        /*
            Не позволяем Windows drive paths:
    
                C:/Windows/...
                C:\Windows\...
        */

        if (relative.has_root_name() || relative.has_root_directory())
            return false;

        const fs::path canonicalRoot =
            fs::weakly_canonical(root);

        std::error_code ec;

        const fs::path candidate =
            fs::weakly_canonical(
                canonicalRoot / relative,
                ec
            );

        if (ec)
            return false;

        /*
            Проверяем, что candidate действительно находится
            внутри canonicalRoot.
        */

        auto rootIt = canonicalRoot.begin();
        auto rootEnd = canonicalRoot.end();

        auto candidateIt = candidate.begin();
        auto candidateEnd = candidate.end();

        for (; rootIt != rootEnd; ++rootIt, ++candidateIt)
        {
            if (candidateIt == candidateEnd)
                return false;

            if (*rootIt != *candidateIt)
                return false;
        }

        result = candidate;

        return true;
    }

}