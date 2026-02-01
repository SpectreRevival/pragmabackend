#pragma once
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <BackendEnvironment.h>
#include <JsonTestUtil.h>

#include "TestHTTPClient.h"
namespace fs = std::filesystem;
using namespace nlohmann;



class HTTPRequestTest : public ::testing::TestWithParam<std::string>
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

TEST_P(HTTPRequestTest, HTTPResponseValidation)
{
    std::ifstream testFile(GetParam());
    std::stringstream ss;
    ss << testFile.rdbuf();
    std::string testJsonStr = ss.str();
    json testJson = json::parse(testJsonStr);
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
    json resJson = json::parse(res.body());
    json expectedResponse = json::parse(testJson["response"].get<std::string>());
    EXPECT_TRUE(JsonMatchesSchema(resJson, expectedResponse,
        testJson.contains("ignoreReplace") && testJson["ignoreReplace"] == true));
}