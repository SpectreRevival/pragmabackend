#pragma once
#include <gtest/gtest.h>
#include <Poco/Process.h>

class BackendEnvironment : public ::testing::Environment {
public:
    std::unique_ptr<Poco::ProcessHandle> server;

    void SetUp() override {
        std::filesystem::remove(std::filesystem::path(SERVER_RUN_DIR) / "playerdata.sqlite");
        const Poco::Process::Args args = {"8081", "8082", "8083"};
        server = std::make_unique<Poco::ProcessHandle>(Poco::Process::launch(SERVER_FILE_PATH, args, SERVER_RUN_DIR));
        std::this_thread::sleep_for(std::chrono::seconds(7));
    }

    void TearDown() override {
        if (server)
        {
            Poco::Process::kill(*server);
            server.reset();
            std::this_thread::sleep_for(std::chrono::seconds(1)); // wait for windows to release file handle
        }
        std::filesystem::remove(std::filesystem::path(SERVER_RUN_DIR) / "playerdata.sqlite");
    }
};