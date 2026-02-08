#include <ResourcesUtilities.h>
#include <iostream>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

namespace fs = std::filesystem;

std::filesystem::path ResourcesUtilities::GetCurrentExecutablePath() {
#if defined(_WIN32)
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer);

#elif defined(__linux__)
    char buffer[4096]; // NOLINT
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1); // NOLINT
    buffer[len] = '\0';
    return std::filesystem::path(buffer);
#else
    static_assert(false, "Unsupported platform");
#endif
}

std::filesystem::path ResourcesUtilities::GetExecutableWorkingDirectory() {
    return ResourcesUtilities::GetCurrentExecutablePath().parent_path();
}

std::filesystem::path ResourcesUtilities::GetResourcesFolder() {
    static std::filesystem::path resFolderPath = [] {
        fs::path current = fs::absolute(GetExecutableWorkingDirectory());

        while (true) {
            fs::path candidate = current / "resources";
            if (fs::is_directory(candidate)) {
                return candidate;
}

            if (current == current.root_path()) {
                std::cerr << "Failed to find resources directory" << '\n';
                throw std::runtime_error("Failed to find resources directory");
            }

            current = current.parent_path();
        }
    }();
    return resFolderPath;
}