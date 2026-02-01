#include <gtest/gtest.h>
#include <filesystem>
#include <WebsocketRequestTest.h>
#include <HTTPRequestTest.h>

static std::vector<std::string> g_ws_paths = []{
    std::vector<std::string> paths;
    for (auto& item : std::filesystem::directory_iterator("../resources/testrequests/ws"))
        {
        if (item.is_regular_file() && item.path().extension() == ".json")
            paths.push_back(item.path().string());
        }
    return paths;
}();

static std::vector<std::string> g_http_paths = []
{
    std::vector<std::string> paths;
    for (auto& item : std::filesystem::directory_iterator("../resources/testrequests/http"))
    {
        if (item.is_regular_file() && item.path().extension() == ".json")
        {
            paths.push_back(item.path().string());
        }
    }
    return paths;
}();

INSTANTIATE_TEST_SUITE_P(WebsocketRequestTests, WebsocketRequestTest, ::testing::ValuesIn(g_ws_paths) );
INSTANTIATE_TEST_SUITE_P(HttpRequestTests, HTTPRequestTest, ::testing::ValuesIn(g_http_paths));

int main(int argc, char** argv)
{
    // Wait for the server to start up
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}