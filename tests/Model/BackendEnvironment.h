#pragma once
#include <gtest/gtest.h>
#include <filesystem>
#include <thread>
#include <process.hpp>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

inline std::filesystem::path getExecutablePath() {
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

class BackendEnvironment : public ::testing::Environment {
public:
    std::unique_ptr<TinyProcessLib::Process> server;

    void SetUp() override {
#if defined(_WIN32)
        std::filesystem::path exePath = getExecutablePath().parent_path() / "..\\" / "pragmabackend.exe";
#elif defined(__linux__)
        std::filesystem::path exePath = getExecutablePath() / "../" / "pragmabackend";
#endif
        std::filesystem::remove(std::filesystem::path(SERVER_RUN_DIR) / "playerdata.sqlite");
        server = std::make_unique<TinyProcessLib::Process>(std::string(std::filesystem::absolute(exePath).string() + " 8081 8082 8083"), SERVER_RUN_DIR,         [](const char *bytes, size_t n) {
            std::cout << std::string(bytes, n);
        },
        [](const char *bytes, size_t n) {
            std::cerr << std::string(bytes, n);
        });
        std::this_thread::sleep_for(std::chrono::seconds(7));
    }

    void TearDown() override {
        if (server)
        {
            server->kill();
            std::this_thread::sleep_for(std::chrono::seconds(1)); // wait for windows to release file handle
            server.reset();
        }
        std::filesystem::remove(std::filesystem::path(SERVER_RUN_DIR) / "playerdata.sqlite");
    }
};