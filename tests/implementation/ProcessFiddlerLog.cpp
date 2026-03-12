#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>
#include <stduuid/uuid.h>
#include <string>

using json = nlohmann::json;
namespace fs = std::filesystem;

bool GetResponseToReqId(int reqId, json& session, json& res) {
    std::string reqIdStr = std::to_string(reqId);
    for (auto message : session["_webSocketMessages"]) {
        if (message["type"] == "send") {
            continue;
        }
        std::string msgData = message["data"];
        if (msgData.find("requestId\":" + reqIdStr) != std::string::npos) {
            res = json::parse(msgData);
            return true;
        }
    }
    return false;
}

fs::path outTestDir;

void ProcessWebsocketSession(json& session) {
    std::random_device rnd;
    std::mt19937 rand(rnd());
    uuids::uuid_random_generator gen(rand);
    // Fiddler annotates from client POV, so send means client send and receive means client receives (server sends)
    for (const auto& message : session["_webSocketMessages"]) {
        if (message["opcode"] != 1) {
            continue;
        }
        if (message["type"] == "receive") {
            continue;
        }
        std::string reqDataStr = message["data"].get<std::string>();
        json reqData = json::parse(reqDataStr);
        int reqId = reqData["requestId"];
        json resData;
        if (!GetResponseToReqId(reqId, session, resData)) {
            continue;
        }
        std::ofstream outTestFile((outTestDir / "ws" / (uuids::to_string(gen()) + ".json")).string());
        json test{};
        test["requestBody"] = reqData["payload"];
        test["rpcType"] = reqData["type"];
        test["responsePayload"] = resData["response"]["payload"];
        test["testAge"] = message["time"];
        outTestFile << test.dump();
        outTestFile.close();
    }
}

std::string ExtractURLPath(const std::string& url) {
    static const std::regex re(R"(https?://[^/]+(/[^?#]*)?)");
    std::smatch match;
    if (std::regex_search(url, match, re)) {
        return match[1].str().empty() ? "/" : match[1].str();
    }
    return {};
}

void ProcessHTTPSession(json& session) {
    std::random_device rnd;
    std::mt19937 rand(rnd());
    uuids::uuid_random_generator gen(rand);
    std::ofstream outTestFile((outTestDir / "http" / ((uuids::to_string(gen()) + ".json"))).string());
    json test{};
    test["method"] = session["request"]["method"];
    test["request"] = session["request"]["postData"]["text"];
    test["path"] = ExtractURLPath(session["request"]["url"].get<std::string>());
    test["response"] = session["response"]["content"]["text"];
    test["testAge"] = session["startedDateTime"];
    outTestFile << test.dump();
    outTestFile.close();
}

int main(int /*argc*/, char** /*argv*/) { // NOLINT
    std::cout << "Enter the path of the fiddler log to open(make sure it's exported as a HTTP archive v1.2): ";
    std::string reqLogPath;
    std::cin >> reqLogPath;
    std::cout << "Enter the path of the directory to place the outputted tests in: ";
    std::cin >> outTestDir;
    fs::create_directories(outTestDir);
    std::ifstream reqLog(reqLogPath);
    if (!reqLog.is_open()) {
        std::cerr << "Failed to open request log" << '\n';
        return -1;
    }
    std::stringstream ss;
    ss << reqLog.rdbuf();
    json reqJson;
    try {
        reqJson = json::parse(ss.str());
    } catch (json::parse_error& e) {
        std::cerr << e.what() << '\n';
        return -1;
    }
    for (auto session : reqJson["log"]["entries"]) {
        if (session.at("request").at("method") == "CONNECT") {
            continue;
        }
        if (session.contains("_webSocketMessages")) {
            std::cout << "Processing WS session" << '\n';
            ProcessWebsocketSession(session);
        } else {
            std::cout << "Processing HTTP Request" << '\n';
            ProcessHTTPSession(session);
        }
    }
}