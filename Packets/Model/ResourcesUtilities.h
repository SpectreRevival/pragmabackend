#pragma once
#include <filesystem>

class ResourcesUtilities
{
public:
    static std::filesystem::path GetResourcesFolder();
    static std::filesystem::path GetCurrentExecutablePath();
    static std::filesystem::path GetExecutableWorkingDirectory();
};