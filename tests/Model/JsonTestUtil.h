#pragma once
#include <nlohmann/json.hpp>
using namespace nlohmann;

bool JsonMatchesSchema(const json& response, const json& expectedResponse, bool ignoreReplace, bool ignoreAdd);