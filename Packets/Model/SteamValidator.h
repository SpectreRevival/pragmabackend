#pragma once
#include <string>
#include <optional>
#include <nlohmann/json.hpp>

struct SteamPlayerInfo {
	std::string steamId;
	std::string personaName;
};

class SteamValidator {
public:
	explicit SteamValidator(std::string apiKey);
	std::optional<SteamPlayerInfo> ValidateSteamId(const std::string& steam64) const;

private:
	std::string m_apiKey;
	static std::string HttpGet(const std::string& host, const std::string& target);
};