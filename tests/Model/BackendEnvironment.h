#pragma once
#include <gtest/gtest.h>
#include <filesystem>
#include <thread>
#include <process.hpp>

class BackendEnvironment : public ::testing::Environment {
public:
    std::unique_ptr<TinyProcessLib::Process> server;

    void SetUp() override {
        std::filesystem::remove(std::filesystem::path(SERVER_RUN_DIR) / "playerdata.sqlite");
        server = std::make_unique<TinyProcessLib::Process>(SERVER_RUN_CMD, SERVER_RUN_DIR,         [](const char *bytes, size_t n) {
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