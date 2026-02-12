#pragma once
#include <RequestTest.h>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::ordered_json;

class WebsocketRequestTest : public RequestTest {
};

void RunWebsocketTest(fs::path testPath, json& outResponse);
void RunWebsocketTest(json testJson, json& outResponse);