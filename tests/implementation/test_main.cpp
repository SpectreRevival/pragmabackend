#include <gtest/gtest.h>
#include <filesystem>
#include <WebsocketRequestTest.h>
#include <HTTPRequestTest.h>

#include "SequencedRequestTest.h"
#include <ResourcesUtilities.h>

static std::vector<std::string> gWsPaths = []{
    std::vector<std::string> paths;
    for (auto& item : std::filesystem::directory_iterator(ResourcesUtilities::GetResourcesFolder() / "testrequests" / "ws"))
        {
        if (item.is_regular_file() && item.path().extension() == ".json") {
            paths.push_back(item.path().string());
}
        }
    return paths;
}();

static std::vector<std::string> gHttpPaths = []
{
    std::vector<std::string> paths;
    for (auto& item : std::filesystem::directory_iterator(ResourcesUtilities::GetResourcesFolder() / "testrequests" / "http"))
    {
        if (item.is_regular_file() && item.path().extension() == ".json")
        {
            paths.push_back(item.path().string());
        }
    }
    return paths;
}();

static std::vector<fs::path> gSequencedDirs = []
{
    std::vector<fs::path> sequencedDirs;
    for (auto& item : std::filesystem::directory_iterator(ResourcesUtilities::GetResourcesFolder() / "testrequests" / "sequenced"))
    {
        if (item.is_directory())
        {
            sequencedDirs.emplace_back(item.path().string());
        }
    }
    return sequencedDirs;
}();

INSTANTIATE_TEST_SUITE_P(WebsocketRequestTests, WebsocketRequestTest, ::testing::ValuesIn(gWsPaths) );
INSTANTIATE_TEST_SUITE_P(HttpRequestTests, HTTPRequestTest, ::testing::ValuesIn(gHttpPaths));
INSTANTIATE_TEST_SUITE_P(SequencedRequestTests, SequencedRequestTest, ::testing::ValuesIn(gSequencedDirs));

int main(int argc, char** argv)
{
    // Wait for the server to start up
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}