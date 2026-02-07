#include <iostream>
#include <ResourcesUtilities.h>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

std::filesystem::path ResourcesUtilities::GetCurrentExecutablePath()
{
#if defined(_WIN32)
        wchar_t buffer[MAX_PATH];
        GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        return std::filesystem::path(buffer);

#elif defined(__linux__)
        char buffer[4096];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        buffer[len] = '\0';
        return std::filesystem::path(buffer);
#else
        static_assert(false, "Unsupported platform");
#endif
}

std::filesystem::path ResourcesUtilities::GetExecutableWorkingDirectory()
{
        return ResourcesUtilities::GetCurrentExecutablePath().parent_path();
}

std::filesystem::path ResourcesUtilities::resourcesFolderPath() {
        static std::filesystem::path resFolderPath = []
        {
                ;
                std::filesystem::path curWorkingDir = GetExecutableWorkingDirectory();
                if (std::filesystem::is_directory(curWorkingDir / "resources"))
                {
                        return curWorkingDir / "resources";
                }
                else if (std::filesystem::is_directory(curWorkingDir / "../" / "resources"))
                {
                        return curWorkingDir / "../" / "resources";
                }
                else
                {
                        std::cerr << "Failed to find resources directory" << std::endl;
                        throw std::runtime_error("Failed to find resources directory");
                }
        }();
        return resFolderPath;
}