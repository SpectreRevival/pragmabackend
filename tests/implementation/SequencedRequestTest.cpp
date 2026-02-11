#include <queue>
#include <SequencedRequestTest.h>
#include <HTTPRequestTest.h>
#include <WebsocketRequestTest.h>
#include <fstream>
#include <vector>

static void ApplyTestRequestInserts(json& payload, const std::vector<json>& responses) {
    for (std::pair<std::string, json> kv: payload) {
        if (!kv.second.is_string()) {
            continue;
        }
        std::string value = kv.second.get<std::string>();
        size_t endPos = value.find('}');
        if (!value.starts_with("${") || endPos == std::string::npos) {
            continue;
        }
        std::string toInterpret = value.substr(3, value.size() - 2 - endPos);
        if (toInterpret.starts_with("responses")) {
            size_t endBrace = toInterpret.find(']');
            int resNumber = stoi(toInterpret.substr(11, toInterpret.size() - endBrace));
            std::string jsonPath = toInterpret.substr(endBrace + 1, toInterpret.size());
            std::vector<std::string> pathSplit; // split at every period
            responses.at(resNumber)
        }
    }
}

TEST_P(SequencedRequestTest, ResponseValidation) {
    std::queue<fs::path> requests;
    std::vector<json> responses;
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
            ApplyTestRequestInserts(testJson["requestBody"], responses);
            json out;
            RunWebsocketTest(testJson, out);
            responses.push_back(out["response"]["payload"]);
        } else {
            // http request
            json out;
            RunHTTPTest(testJson, out);
            responses.push_back(out);
        }
    }
}