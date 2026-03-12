#include "SpectreRpcType.h"

#include <LoginToChatProcessor.h>
#include <VivoxTokenGenerator.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

struct VivoxConfig {
    std::string domain;
    std::string issuer;
    std::string key;
    std::string server;
};

static std::optional<VivoxConfig> LoadVivoxCfg() {
    VivoxConfig cfg;

    try {
        std::ifstream f("auth.json");

        nlohmann::json j;
        f >> j;
        const auto& v = j["vivox"];

        cfg.domain = v.value("domain", "");
        cfg.issuer = v.value("issuer", "");
        cfg.key = v.value("key", "");
        cfg.server = v.value("server", "");

    } catch ([[maybe_unused]] const std::exception& e) {
        spdlog::warn("failed to parse auth.json, probably missing cfg");
        return std::nullopt;
    }

    return cfg;
}

LoginToChatProcessor::LoginToChatProcessor(const SpectreRpcType& rpcType) : WebsocketPacketProcessor(rpcType) {}

void LoginToChatProcessor::Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) {
    const auto cfgOpt = LoadVivoxCfg();
    if (!cfgOpt.has_value()) {
        spdlog::warn("vivox not set up, not logging into voice chat");
        return;
    }
    const VivoxConfig& cfg = cfgOpt.value();
    const std::string playerId = sock.GetPlayerId();

    const std::shared_ptr<json> response = packet.GetBaseJsonResponse();
    json& payload = (*response)["payload"];

    const std::string loginToken = VivoxTokenGenerator::Generate(
        cfg.key,
        cfg.issuer,
        cfg.domain,
        playerId,
        "login");

    const std::string joinToken = VivoxTokenGenerator::Generate(
        cfg.key,
        cfg.issuer,
        cfg.domain,
        playerId,
        "join",
        "global");

    payload["success"] = true;
    payload["token"] = loginToken;

    payload["generalchatroom"] = "global";
    payload["generalchattoken"] = joinToken;

    payload["chatserver"] = cfg.server;
    payload["chatdomain"] = cfg.domain;
    payload["chatissuer"] = cfg.issuer;

    // spdlog::info("vivox login gen for player {}", playerId);
    sock.SendPacket(response);
}