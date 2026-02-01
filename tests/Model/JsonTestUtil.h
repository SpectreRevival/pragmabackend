#pragma once
#include <iostream>
#include <nlohmann/json.hpp>
using namespace nlohmann;

inline bool JsonMatchesSchema(const json& response, const json& expectedResponse, bool ignoreReplace)
{
    json diff = json::diff(expectedResponse, response);
    std::cout << "DIFF: " << std::endl;
    std::cout << diff.dump() << std::endl;
    // Have to do it this way for the * check to work, but the source expectedResponse one is printed because it makes more sense
    json testDiff = json::diff(response, expectedResponse);
    for (const auto& entry : testDiff)
    {
        // each entry has 3 fields: op, path, and value.
        if (entry["op"] != "replace")
        {
            return false;
        }
        if (entry["value"] != "*" && !ignoreReplace)
        {
            return false;
        }
    }
    return true;
}