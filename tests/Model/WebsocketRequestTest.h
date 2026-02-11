#pragma once
#include <nlohmann/json.hpp>
#include <RequestTest.h>

namespace fs = std::filesystem;
using namespace nlohmann;

class WebsocketRequestTest : public RequestTest {
};

void RunWebsocketTest(fs::path testPath);
void RunWebsocketTest(json testJson);