#include <queue>
#include <SequencedRequestTest.h>
#include <HTTPRequestTest.h>
#include <WebsocketRequestTest.h>
#include <fstream>
#include <vector>
#include <ranges>

static std::vector<std::string> SplitString(std::string_view s, char delim) {
    std::vector<std::string> out;

    for (auto part : std::views::split(s, delim)) {
        out.emplace_back(part.begin(), part.end());
    }

    return out;
}

static void ApplyTestRequestInserts(json& payload, const std::vector<json>& responses) {
    for (auto& [key, value]: payload.items()) {
        if (!value.is_string()) {
            continue;
        }
        std::string expression = value.get<std::string>();
        size_t endPos = expression.find('}');
        if (!expression.starts_with("${") || endPos == std::string::npos) {
            continue;
        }
        std::string toInterpret = expression.substr(3, value.size() - 2 - endPos);
        if (toInterpret.starts_with("responses")) {
            size_t endBrace = toInterpret.find(']');
            int resNumber = stoi(toInterpret.substr(11, toInterpret.size() - endBrace));
            std::string jsonPath = toInterpret.substr(endBrace + 1, toInterpret.size());
            std::vector<std::string> pathSplit = SplitString(jsonPath, '.');
            json valueToEmplace = responses.at(resNumber).at(pathSplit.front());
            for (int i = 1; i < pathSplit.size(); i++) {
                valueToEmplace = valueToEmplace.at(pathSplit.at(i));
            }
            payload[key] = valueToEmplace;
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