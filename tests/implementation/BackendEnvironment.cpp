#include <BackendEnvironment.h>
#include <ResourcesUtilities.h>
#include <filesystem>
#include <thread>

#include "PersistenceUtilities.h"

void BackendEnvironment::SetUp() {
#if defined(_WIN32)
    std::filesystem::path exePath = ResourcesUtilities::GetCurrentExecutablePath().parent_path().parent_path() /
                                    "pragmabackend.exe";
    std::system("taskkill /f /im pragmabackend.exe"); // NOLINT
#elif defined(__linux__)
    std::filesystem::path exePath = ResourcesUtilities::GetCurrentExecutablePath().parent_path().parent_path() /
                                    "pragmabackend";
    std::system("pkill -9 pragmabackend"); // NOLINT
#endif
    std::filesystem::remove(PersistenceUtilities::GetSavePath() / "playerdata.sqlite");
    server = std::make_unique<TinyProcessLib::Process>(
        std::string(std::filesystem::absolute(exePath).string() + " 8081 8082 8083"),
        ResourcesUtilities::GetCurrentExecutablePath().parent_path().parent_path().string(),
        [](const char* bytes, size_t n) {
            std::cout << std::string(bytes, n);
        },
        [](const char* bytes, size_t n) {
            std::cerr << std::string(bytes, n);
        });
    std::this_thread::sleep_for(std::chrono::seconds(7));
}

void BackendEnvironment::TearDown() {
    if (server) {
        server->kill();
        std::this_thread::sleep_for(std::chrono::seconds(1)); // wait for windows to release file handle
        server.reset();
    }
    std::filesystem::remove(PersistenceUtilities::GetSavePath() / "playerdata.sqlite");
}