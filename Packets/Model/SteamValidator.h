#pragma once
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

struct SteamPlayerInfo {
    std::string steamId;
    std::string personaName;
};

class SteamValidator {
  public:
    explicit SteamValidator(std::string apiKey);
    [[nodiscard]] std::optional<SteamPlayerInfo> ValidateSteamId(const std::string& steam64) const;

  private:
    std::string apiKey;
    static std::string HttpGet(const std::string& host, const std::string& target);
};