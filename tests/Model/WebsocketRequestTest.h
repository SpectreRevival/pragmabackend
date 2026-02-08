#pragma once
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <TestWebsocketClient.h>
#include <BackendEnvironment.h>
#include <JsonTestUtil.h>

#include "boost/asio/buffers_iterator.hpp"
namespace fs = std::filesystem;
using namespace nlohmann;

class WebsocketRequestTest : public ::testing::TestWithParam<std::string>
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

TEST_P(WebsocketRequestTest, ResponseValidation)
{
    std::ifstream testFile(GetParam());
    std::stringstream ss;
    ss << testFile.rdbuf();
    std::string testJsonStr = ss.str();
    json testJson = json::parse(testJsonStr);
    TestWebsocketClient wsClient(8083);
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
}