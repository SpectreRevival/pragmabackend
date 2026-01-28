#pragma once
#include <boost/process.hpp>
#include <gtest/gtest.h>

namespace proc   = boost::process;
namespace asio   = boost::asio;
// todo wipe playerdata.sqlite during setup and teardown, as well as moving the instantiation of the server process to SetUp instead of instantiation time

class BackendEnvironment : public ::testing::Environment {
public:
    asio::io_context ctx;
    std::unique_ptr<proc::process> server;

    void SetUp() override {
#if WIN32
        std::system("taskkill /f /im pragmabackend.exe");
#else
        std::system("pkill -6 pragmabackend");
#endif
        std::filesystem::remove("../playerdata.sqlite");
        server = std::make_unique<proc::process>(
            ctx,
            proc::environment::find_executable(SERVER_FILE_PATH),
            std::vector<std::string>{"8081", "8082", "8083"},
            proc::process_start_dir{SERVER_RUN_DIR}
        );
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    void TearDown() override {
        if (server->running())
            server->terminate();
        server->wait();
        server.reset();
        std::filesystem::remove(std::filesystem::path(SERVER_RUN_DIR) / "playerdata.sqlite");
    }
};