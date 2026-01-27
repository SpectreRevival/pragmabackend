#include <gtest/gtest.h>
#include <filesystem>
namespace fs = std::filesystem;

TEST(WebsocketSchemasMatch, WebsocketRequests)
{
    for (auto item : fs::directory_iterator("../resources/testrequests/ws"))
    {

    }
}