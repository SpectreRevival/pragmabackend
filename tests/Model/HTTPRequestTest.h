#pragma once
#include <RequestTest.h>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::ordered_json;

class HTTPRequestTest : public RequestTest {
};

void RunHTTPTest(const fs::path& testPath, json& outResponse);
void RunHTTPTest(json testJson, json& outResponse);