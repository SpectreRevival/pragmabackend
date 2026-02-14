#include <WebsocketRequestTest.h>
#include <fstream>
#include <JsonTestUtil.h>
#include <TestWebsocketClient.h>
#include "boost/asio/buffers_iterator.hpp"

void RunWebsocketTest(const fs::path& testJsonPath, json& outResponse)
{
    std::ifstream testFile(testJsonPath);
    std::stringstream ss;
    ss << testFile.rdbuf();
    std::string testJsonStr = ss.str();
    json testJson = json::parse(testJsonStr);
    RunWebsocketTest(testJson, outResponse);
}

void RunWebsocketTest(json testJson, json& outResponse) // NOLINT
{
    TestWebsocketClient wsClient(8082);
    SpectreRpcType reqType = SpectreRpcType(testJson.at("rpcType").get<std::string>());
    std::cout << "Test info: " << '\n';
    std::cout << "RPC type: " << reqType.GetName() << '\n';
    std::cout << "Request payload: " << testJson.at("requestBody").dump() << '\n';
    std::cout << "Expected response payload: " << testJson.at("responsePayload").dump() << '\n';
    json& reqBody = testJson.at("requestBody");
    boost::beast::flat_buffer res = wsClient.SendPacket(reqBody, reqType);
    std::string resStr(boost::asio::buffers_begin(res.data()), boost::asio::buffers_end(res.data()));
    if (resStr.empty()) {
        GTEST_SKIP() << "Skipping since no response given";
    }
    ASSERT_NO_THROW(outResponse = json::parse(resStr));
    ASSERT_TRUE(outResponse.contains("sequenceNumber"));
    ASSERT_TRUE(outResponse.at("sequenceNumber") == 0);
    ASSERT_TRUE(outResponse.contains("response"));
    ASSERT_TRUE(outResponse.at("response").contains("requestId"));
    ASSERT_TRUE(outResponse.at("response").contains("type"));
    ASSERT_TRUE(SpectreRpcType(outResponse["response"]["type"].get<std::string>()) == reqType.GetResponseType());
    ASSERT_TRUE(outResponse.at("response").contains("payload"));
    ASSERT_TRUE(JsonMatchesSchema(outResponse.at("response").at("payload"), testJson.at("responsePayload"),
                                  testJson.contains("ignoreReplace") && testJson.at("ignoreReplace") == true,
                                  !testJson.contains("ignoreAdd") || testJson.at("ignoreAdd") == true));
}

TEST_P(WebsocketRequestTest, ResponseValidation) {
    json out;
    RunWebsocketTest(GetParam(), out);
}