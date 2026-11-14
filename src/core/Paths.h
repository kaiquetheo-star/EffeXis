#pragma once

#include <filesystem>


namespace core
{
    // Folder of the settings and caches of EffeXis, where the system keeps
    // those of the applications: %APPDATA%\EffeXis on Windows,
    // $XDG_CONFIG_HOME/effexis (~/.config/effexis) elsewhere. Created
    // when missing.
    std::filesystem::path configDirectory();

    // Folder of the files EffeXis makes again when they are missing (the
    // pipeline cache of the GPU): %LOCALAPPDATA%\EffeXis\Cache on Windows,
    // $XDG_CACHE_HOME/effexis (~/.cache/effexis) elsewhere. Created when
    // missing.
    std::filesystem::path cacheDirectory();
}
