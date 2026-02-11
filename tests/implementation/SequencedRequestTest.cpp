#include <queue>
#include <SequencedRequestTest.h>
#include <HTTPRequestTest.h>
#include <WebsocketRequestTest.h>
#include <fstream>

TEST_P(SequencedRequestTest, ResponseValidation) {
    std::queue<fs::path> requests;
    int i = 0;
    while (true) {
        fs::path curTestPath = GetParam() / (std::to_string(i) + ".json");
        if (fs::exists(curTestPath)) {
            requests.push(curTestPath);
        } else {
            break;
        }
        i++;
    }
    while (true) {
        if (requests.empty()) {
            break;
        }
        fs::path curTestPath = requests.front();
        requests.pop();
        std::ifstream testJsonF(curTestPath);
        std::stringstream ss;
        ss << testJsonF.rdbuf();
        std::string testJsonStr = ss.str();
        json testJson = json::parse(testJsonStr);
        if (testJson.contains("rpcType")) {
            // ws request
            RunWebsocketTest(testJson);
        } else {
            // http request
            RunHTTPTest(testJson);
        }
    }
}