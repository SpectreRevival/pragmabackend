#include <PersistenceUtilities.h>
#include <spdlog/spdlog.h>
#ifdef _WIN32
#include <shlobj.h>
#include <windows.h>
#endif

#ifdef _WIN32
static std::filesystem::path GetAppDataPath_win() {
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path))) {
        char buffer[MAX_PATH];
        size_t converted = 0;
        wcstombs_s(&converted, buffer, MAX_PATH, path, _TRUNCATE);
        CoTaskMemFree(path);
        return std::filesystem::path(buffer);
    }
    spdlog::error("Failed to get roaming appdata path");
    throw std::runtime_error("Failed to get roaming appdata path");
}
#endif
#ifdef __linux__
static std::filesystem::path GetAppDataPathLinux() {
    const char* xdg = std::getenv("XDG_DATA_HOME");
    if ((xdg != nullptr) && (*xdg != 0)) {
        return std::filesystem::path(xdg);
    }
    const char* home = std::getenv("HOME");
    if ((home != nullptr) && (*home != 0)) {
        return std::filesystem::path(home) / ".local" / "share";
    }
    spdlog::error("Failed to find XDG_DATA_HOME or HOME directory to save data");
    throw std::runtime_error("Failed to find XDG_DATA_HOME or HOME directory to save data");
}
#endif

std::filesystem::path PersistenceUtilities::GetAppDataPath() {
#ifdef _WIN32
    return GetAppDataPath_win();
#endif
#ifdef __linux__
    return GetAppDataPathLinux();
#endif
}

std::filesystem::path PersistenceUtilities::GetSavePath() {
    std::filesystem::path savePath = GetAppDataPath() / "pragmabackend";
    std::filesystem::create_directories(savePath);
    return savePath;
}