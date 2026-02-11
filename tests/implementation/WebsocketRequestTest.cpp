#include <WebsocketRequestTest.h>
#include <fstream>
#include <JsonTestUtil.h>
#include <TestWebsocketClient.h>
#include "boost/asio/buffers_iterator.hpp"

void RunWebsocketTest(fs::path testJsonPath, json& outResponse)
{
    std::ifstream testFile(testJsonPath);
    std::stringstream ss;
    ss << testFile.rdbuf();
    std::string testJsonStr = ss.str();
    json testJson = json::parse(testJsonStr);
    RunWebsocketTest(testJson, outResponse);
}

void RunWebsocketTest(json testJson, json& outResponse)
{
    TestWebsocketClient wsClient(8082);
    SpectreRpcType reqType = SpectreRpcType(testJson["rpcType"].get<std::string>());
    std::cout << "Test info: " << std::endl;
    std::cout << "RPC type: " << reqType.GetName() << std::endl;
    std::cout << "Request payload: " << testJson["requestBody"].dump() << std::endl;
    std::cout << "Expected response payload: " << testJson["responsePayload"].dump() << std::endl;
    boost::beast::flat_buffer res = wsClient.SendPacket(testJson["requestBody"], reqType);
    std::string resStr(boost::asio::buffers_begin(res.data()), boost::asio::buffers_end(res.data()));
    if (resStr.empty()) {
        GTEST_SKIP() << "Skipping since no response given";
    }
    ASSERT_NO_THROW(outResponse = json::parse(resStr));
    ASSERT_TRUE(outResponse.contains("sequenceNumber"));
    ASSERT_TRUE(outResponse["sequenceNumber"] == 0);
    ASSERT_TRUE(outResponse.contains("response"));
    ASSERT_TRUE(outResponse["response"].contains("requestId"));
    ASSERT_TRUE(outResponse["response"].contains("type"));
    ASSERT_TRUE(SpectreRpcType(outResponse["response"]["type"].get<std::string>()) == reqType.GetResponseType());
    ASSERT_TRUE(outResponse["response"].contains("payload"));
    ASSERT_TRUE(JsonMatchesSchema(outResponse["response"]["payload"], testJson["responsePayload"],
                                  testJson.contains("ignoreReplace") && testJson["ignoreReplace"] == true,
                                  !testJson.contains("ignoreAdd") || testJson["ignoreAdd"] == true));
}

TEST_P(WebsocketRequestTest, ResponseValidation) {
    json out;
    RunWebsocketTest(GetParam(), out);
}