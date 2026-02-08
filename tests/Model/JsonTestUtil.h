#pragma once
#include <iostream>
#include <nlohmann/json.hpp>
using namespace nlohmann;

inline bool JsonMatchesSchema(const json& response, const json& expectedResponse, bool ignoreReplace, bool ignoreAdd) {
    json diff = json::diff(expectedResponse, response);
    std::cout << "DIFF: " << std::endl;
    std::cout << diff.dump() << std::endl;
    // Have to do it this way for the * check to work, but the source expectedResponse one is printed because it makes more sense
    json testDiff = json::diff(response, expectedResponse);
    for (const auto& entry : testDiff) // NOLINT(*-use-anyofallof)
    {
        // Note that remove and add are flipped from what you would think here. Remove means it's not in the test but is in the response.
        // This is okay since I assume that the game doesn't use very strict json deserialization so additional fields will just go unchecked.
        if (entry["op"] == "remove") {
            if (ignoreAdd) {
                continue;
            }
            return false;
        }
        // This is stuff that is in the test but not in the response, very bad
        if (entry["op"] == "add") {
            return false;
        }
        if (entry["value"] != "*" && !ignoreReplace) {
            return false;
        }
    }
    return true;
}