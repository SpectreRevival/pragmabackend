#pragma once
#include <filesystem>

class ResourcesUtilities
{
public:
    static std::filesystem::path resourcesFolderPath();
    static std::filesystem::path GetCurrentExecutablePath();
    static std::filesystem::path GetExecutableWorkingDirectory();
};