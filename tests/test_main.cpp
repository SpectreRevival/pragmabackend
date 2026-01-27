#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--gtest_list_tests")
        {
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        }
    }
    std::thread serverThread = std::thread([]{std::system(SERVER_RUN_CMD);});
    // Wait for the server to start up
    std::this_thread::sleep_for(std::chrono::seconds(10));
    ::testing::InitGoogleTest(&argc, argv);
    int retCode = RUN_ALL_TESTS();
    std::system(SERVER_TASKKILL_CMD);
    serverThread.join();
    return retCode;
}