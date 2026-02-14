#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <chrono>
#include <iomanip>
#include <fstream>

#include <SpectreRpcType.h>
namespace fs = std::filesystem;
using json = nlohmann::json;

void DeduplicateWebsocketRequests(const std::string& reqFolder)
{
    std::unordered_map<SpectreRpcType, std::pair<fs::path, double>> filesToKeep;
    for (const auto& item : fs::directory_iterator(reqFolder))
    {
        if (!item.is_regular_file())
        {
            continue;
        }
        if (item.path().extension() != ".json")
        {
            continue;
        }
        std::ifstream testFile(item.path().string());
        std::stringstream ss;
        ss << testFile.rdbuf();
        std::string testFileStr = ss.str();
        json testJson = json::parse(testFileStr);
        SpectreRpcType rpcType = SpectreRpcType(testJson["rpcType"].get<std::string>());
        double t = testJson["testAge"];
        if (filesToKeep.contains(rpcType))
        {
            if (filesToKeep.at(rpcType).second < t)
            {
                filesToKeep[rpcType] = {item.path(), t};
            }
        } else
        {
            filesToKeep[rpcType] = {item.path(), t};
        }
    }
    for (auto const& item : fs::directory_iterator(reqFolder))
    {
        if (!item.is_regular_file())
        {
            continue;
        }
        if (item.path().extension() != ".json")
        {
            continue;
        }
        bool keep = std::ranges::any_of(
            filesToKeep | std::views::values,
            [&](auto const& entry) {
                return entry.first == item.path();
            }
        );

        if (keep) {
            continue;
}
        fs::remove(item);
    }
}

int64_t ParseTimestamp(std::string s)
{
    s = s.substr(0, 19);

    std::tm tm{};
    std::istringstream ss(s);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    int64_t unixTime = 0;
#if _WIN32
    unixTime = _mkgmtime(&tm);
#else
    unixTime = timegm(&tm);
#endif
    return unixTime;
}


void DeduplicateHTTPRequests(const std::string& reqFolder)
{
    std::unordered_map<std::string, std::pair<fs::path, int64_t>> filesToKeep;
    for (const auto& item : fs::directory_iterator(reqFolder))
    {
        if (!item.is_regular_file())
        {
            continue;
        }
        if (item.path().extension() != ".json")
        {
            continue;
        }
        std::ifstream testFile(item.path().string());
        std::stringstream ss;
        ss << testFile.rdbuf();
        std::string testFileStr = ss.str();
        json testJson = json::parse(testFileStr);
        std::string path = testJson["path"];
        int64_t timeUnix = ParseTimestamp(testJson["testAge"].get<std::string>());
        if (filesToKeep.contains(path))
        {
            if (filesToKeep.at(path).second < timeUnix)
            {
                filesToKeep[path] = {item.path(), timeUnix};
            }
        } else
        {
            filesToKeep[path] = {item.path(), timeUnix};
        }
    }
    for (auto const& item : fs::directory_iterator(reqFolder))
    {
        if (!item.is_regular_file())
        {
            continue;
        }
        if (item.path().extension() != ".json")
        {
            continue;
        }
        bool keep = std::ranges::any_of(
            filesToKeep | std::views::values,
            [&](auto const& entry) {
                return entry.first == item.path();
            }
        );

        if (keep) {
            continue;
}
        fs::remove(item);
    }
}

int main() // NOLINT
{
    std::string reqFolder;
    std::cout << "Enter the folder with the requests: ";
    std::cin >> reqFolder;
    std::string reqType;
    std::cout << "Enter the request type: (w or h): ";
    std::cin >> reqType;
    if (reqType == "w")
    {
        DeduplicateWebsocketRequests(reqFolder);
    } else if (reqType == "h")
    {
        DeduplicateHTTPRequests(reqFolder);
    } else
    {
        std::cout << "Unknown option entered" << '\n';
        return -1;
    }
}