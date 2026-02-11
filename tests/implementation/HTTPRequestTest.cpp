#include <HTTPRequestTest.h>
#include <fstream>
#include <boost/beast/http.hpp>
#include <TestHTTPClient.h>
#include <JsonTestUtil.h>

void RunHTTPTest(fs::path testPath)
{
    std::ifstream testFile(testPath);
    std::stringstream ss;
    ss << testFile.rdbuf();
    std::string testJsonStr = ss.str();
    json testJson = json::parse(testJsonStr);
    RunHTTPTest(testJson);
}


void RunHTTPTest(json testJson)
{
    std::cout << "Test info: " << std::endl;
    std::cout << "Path: " << testJson["path"].dump() << std::endl;
    std::cout << "method: " << testJson["method"].dump() << std::endl;
    std::cout << "request payload: " << testJson["request"].dump() << std::endl;
    std::cout << "response payload: " << testJson["response"].dump() << std::endl;
    boost::beast::http::response<boost::beast::http::string_body> res;
    if (testJson["method"] == "GET") {
        res = HTTPFetch(8081, testJson["path"], testJson["request"], boost::beast::http::verb::get);
    } else if (testJson["method"] == "POST") {
        res = HTTPFetch(8081, testJson["path"], testJson["request"], boost::beast::http::verb::post);
    } else {
        std::cerr << "Unrecognized http verb: " << testJson["method"] << std::endl;
        GTEST_FATAL_FAILURE_("Unrecognized http verb");
    }
    if (res.body().empty()) {
        GTEST_SKIP() << "Got an empty response";
    }
    json resJson = json::parse(res.body());
    json expectedResponse = json::parse(testJson["response"].get<std::string>());
    EXPECT_TRUE(JsonMatchesSchema(resJson, expectedResponse,
                                  testJson.contains("ignoreReplace") && testJson["ignoreReplace"] == true,
                                  !testJson.contains("ignoreAdd") || testJson["ignoreAdd"] == true));
}

TEST_P(HTTPRequestTest, ResponseValidation) {
    RunHTTPTest(GetParam());
}