#pragma once
#include <nlohmann/json.hpp>
#include <RequestTest.h>

namespace fs = std::filesystem;
using namespace nlohmann;

class HTTPRequestTest : public RequestTest
{

};

void RunHTTPTest(fs::path testPath);
void RunHTTPTest(json testJson);