#pragma once
#include <ResourcesUtilities.h>
#include <filesystem>
#include <gtest/gtest.h>
#include <process.hpp>
#include <thread>

class BackendEnvironment : public ::testing::Environment
{
  public:
    std::unique_ptr<TinyProcessLib::Process> server;

    void SetUp() override
    {
#if defined(_WIN32)
        std::filesystem::path exePath =
            ResourcesUtilities::GetCurrentExecutablePath()
                .parent_path()
                .parent_path() /
            "pragmabackend.exe";
#elif defined(__linux__)
        std::filesystem::path exePath =
            ResourcesUtilities::GetCurrentExecutablePath()
                .parent_path()
                .parent_path() /
            "pragmabackend";
#endif
        std::filesystem::remove(ResourcesUtilities::GetCurrentExecutablePath()
                                    .parent_path()
                                    .parent_path() /
                                "playerdata.sqlite");
        server = std::make_unique<TinyProcessLib::Process>(
            std::string(std::filesystem::absolute(exePath).string() +
                        " 8081 8082 8083"),
            ResourcesUtilities::GetCurrentExecutablePath()
                .parent_path()
                .parent_path()
                .string(),
            [](const char *bytes, size_t n)
            { std::cout << std::string(bytes, n); },
            [](const char *bytes, size_t n)
            { std::cerr << std::string(bytes, n); });
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    void TearDown() override
    {
        if (server)
        {
            server->kill();
            std::this_thread::sleep_for(std::chrono::seconds(
                1)); // wait for windows to release file handle
            server.reset();
        }
        std::filesystem::remove(ResourcesUtilities::GetCurrentExecutablePath()
                                    .parent_path()
                                    .parent_path() /
                                "playerdata.sqlite");
    }
};
