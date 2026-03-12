#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

bool JsonMatchesSchema(const json& response, const json& expectedResponse, bool ignoreReplace, bool ignoreAdd);