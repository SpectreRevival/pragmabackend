#pragma once
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <TestWebsocketClient.h>
#include <BackendEnvironment.h>

#include "boost/asio/buffers_iterator.hpp"
namespace fs = std::filesystem;
using namespace nlohmann;

inline bool JsonMatchesSchema(const json& response, const json& payload)
{
    json diff = json::diff(response, payload);
    for (const auto& entry : diff)
    {
        // each entry has 3 fields: op, path, and value.
        if (entry["op"] != "replace")
        {
            return false;
        }
        if (entry["value"] != "*")
        {
            return false;
        }
    }
    return true;
}

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

TEST_P(WebsocketRequestTest, WebsocketResponseValidation)
{
    std::ifstream testFile(GetParam());
    std::stringstream ss;
    ss << testFile.rdbuf();
    std::string testJsonStr = ss.str();
    json testJson = json::parse(testJsonStr);
    TestWebsocketClient wsClient(80);
    SpectreRpcType reqType = SpectreRpcType(testJson["rpcType"].get<std::string>());
    boost::beast::flat_buffer res = wsClient.SendPacket(testJson["requestBody"], reqType);
    std::string resStr( boost::asio::buffers_begin(res.data()), boost::asio::buffers_end(res.data()) );
    json responseFull;
    ASSERT_NO_THROW(responseFull = json::parse(resStr));
    ASSERT_TRUE(responseFull.contains("sequenceNumber"));
    ASSERT_TRUE(responseFull["sequenceNumber"] == 0);
    ASSERT_TRUE(responseFull.contains("response"));
    ASSERT_TRUE(responseFull["response"].contains("requestId"));
    ASSERT_TRUE(responseFull["response"].contains("type"));
    ASSERT_TRUE(SpectreRpcType(responseFull["response"]["type"].get<std::string>()) == reqType.GetResponseType());
    ASSERT_TRUE(responseFull["response"].contains("payload"));
    ASSERT_TRUE(JsonMatchesSchema(responseFull["response"]["payload"], testJson["responsePayload"]));
}