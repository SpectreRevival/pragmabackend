#pragma once
#include <boost/process.hpp>
#include <gtest/gtest.h>

namespace proc   = boost::process;
namespace asio   = boost::asio;
// todo wipe playerdata.sqlite during setup and teardown, as well as moving the instantiation of the server process to SetUp instead of instantiation time

class BackendEnvironment : public ::testing::Environment {
public:
    asio::io_context ctx;
    proc::process server{ctx, proc::environment::find_executable(SERVER_FILE_PATH),
        // game port, social port, ws port (changed from default ws port of 80 for permissions reasons on ci server)
    {"8081", "8082", "8083"},
    proc::process_start_dir{SERVER_RUN_DIR}};

    void SetUp() override {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    void TearDown() override {
        if (server.running())
            server.terminate();
        server.wait();
    }
};