#pragma once
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <TestWebsocketClient.h>
#include <BackendEnvironment.h>
#include <JsonTestUtil.h>
#include <queue>

#include "boost/asio/buffers_iterator.hpp"

namespace fs = std::filesystem;
using namespace nlohmann;

class SequencedRequestTest : public ::testing::TestWithParam<fs::path>
{
    std::unique_ptr<BackendEnvironment> backend;

    void SetUp() override
    {
        backend = std::make_unique<BackendEnvironment>();
        backend->SetUp();
    }

    void TearDown() override
    {
        backend->TearDown();
        backend.reset();
    }
};

TEST_P(SequencedRequestTest, ResponseValidation)
{
    std::queue<fs::path> requests;
    int i = 0;
    while (true)
    {
        fs::path curTestPath = GetParam() / (std::to_string(i) + ".json");
        if (fs::exists(curTestPath))
        {
            requests.push(curTestPath);
        } else
        {
            break;
        }
        i++;
    }
    TestWebsocketClient wsClient(8083);
    while (true)
    {
        if (requests.empty())
        {
            break;
        }
        fs::path curTestPath = requests.front();
        requests.pop();
        std::ifstream testJsonF(curTestPath);
        std::stringstream ss;
        ss << testJsonF.rdbuf();
        std::string testJsonStr = ss.str();
        json testJson = json::parse(testJsonStr);
        if (testJson.contains("rpcType"))
        {
            // ws request
            SpectreRpcType reqType = SpectreRpcType(testJson["rpcType"].get<std::string>());
            std::cout << "Test info: " << std::endl;
            std::cout << "RPC type: " << reqType.GetName() << std::endl;
            std::cout << "Request payload: " << testJson["requestBody"].dump() << std::endl;
            std::cout << "Expected response payload: " << testJson["responsePayload"].dump() << std::endl;
            boost::beast::flat_buffer res = wsClient.SendPacket(testJson["requestBody"], reqType);
            std::string resStr( boost::asio::buffers_begin(res.data()), boost::asio::buffers_end(res.data()) );
            if (resStr.empty())
            {
                GTEST_SKIP() << "Skipping since no response given";
            }
            json responseFull;
            ASSERT_NO_THROW(responseFull = json::parse(resStr));
            ASSERT_TRUE(responseFull.contains("sequenceNumber"));
            ASSERT_TRUE(responseFull["sequenceNumber"] == 0);
            ASSERT_TRUE(responseFull.contains("response"));
            ASSERT_TRUE(responseFull["response"].contains("requestId"));
            ASSERT_TRUE(responseFull["response"].contains("type"));
            ASSERT_TRUE(SpectreRpcType(responseFull["response"]["type"].get<std::string>()) == reqType.GetResponseType());
            ASSERT_TRUE(responseFull["response"].contains("payload"));
            ASSERT_TRUE(JsonMatchesSchema(responseFull["response"]["payload"], testJson["responsePayload"],
                testJson.contains("ignoreReplace") && testJson["ignoreReplace"] == true,
                !testJson.contains("ignoreAdd") || testJson["ignoreAdd"] == true));
        } else
        {
            // http request
            std::cout << "Test info: " << std::endl;
            std::cout << "Path: " << testJson["path"].dump() << std::endl;
            std::cout << "method: " << testJson["method"].dump() << std::endl;
            std::cout << "request payload: " << testJson["request"].dump() << std::endl;
            std::cout << "response payload: " << testJson["response"].dump() << std::endl;
            boost::beast::http::response<boost::beast::http::string_body> res;
            if (testJson["method"] == "GET")
            {
                res = HTTPFetch(8081, testJson["path"], testJson["request"], boost::beast::http::verb::get);
            } else if (testJson["method"] == "POST")
            {
                res = HTTPFetch(8081, testJson["path"], testJson["request"], boost::beast::http::verb::post);
            } else
            {
                std::cerr << "Unrecognized http verb: " << testJson["method"] << std::endl;
                GTEST_FATAL_FAILURE_("Unrecognized http verb");
            }
            if (res.body().empty())
            {
                GTEST_SKIP() << "Got an empty response";
            }
            json resJson = json::parse(res.body());
            json expectedResponse = json::parse(testJson["response"].get<std::string>());
            EXPECT_TRUE(JsonMatchesSchema(resJson, expectedResponse,
                testJson.contains("ignoreReplace") && testJson["ignoreReplace"] == true,
                !testJson.contains("ignoreAdd") || testJson["ignoreAdd"] == true));
        }
    }
}
