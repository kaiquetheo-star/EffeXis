#include <core/Paths.h>

#include <cstdlib>
#include <system_error>


namespace core
{

std::filesystem::path configDirectory()
{
    std::filesystem::path folder;

#ifdef _WIN32
    if (const wchar_t* appData = _wgetenv(L"APPDATA")) {
        folder = std::filesystem::path(appData) / "EffeXis";
    }
#else
    if (const char* config = std::getenv("XDG_CONFIG_HOME"); config && *config) {
        folder = std::filesystem::path(config) / "effexis";
    }
    else if (const char* home = std::getenv("HOME")) {
        folder = std::filesystem::path(home) / ".config" / "effexis";
    }
#endif

    if (folder.empty()) {
        folder = std::filesystem::temp_directory_path() / "EffeXis";
    }

    std::error_code error;
    std::filesystem::create_directories(folder, error);

    return folder;
}


std::filesystem::path cacheDirectory()
{
    std::filesystem::path folder;

#ifdef _WIN32
    if (const wchar_t* localAppData = _wgetenv(L"LOCALAPPDATA")) {
        folder = std::filesystem::path(localAppData) / "EffeXis" / "Cache";
    }
#else
    if (const char* cache = std::getenv("XDG_CACHE_HOME"); cache && *cache) {
        folder = std::filesystem::path(cache) / "effexis";
    }
    else if (const char* home = std::getenv("HOME")) {
        folder = std::filesystem::path(home) / ".cache" / "effexis";
    }
#endif

    if (folder.empty()) {
        folder = std::filesystem::temp_directory_path() / "EffeXis";
    }

    std::error_code error;
    std::filesystem::create_directories(folder, error);

    return folder;
}

}
