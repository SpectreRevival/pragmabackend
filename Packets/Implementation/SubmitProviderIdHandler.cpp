#include "AuthLatch.h"
#include "AuthenticateHandler.h"
#include "ResourcesUtilities.h"

#include <SteamValidator.h>
#include <SubmitProviderIdHandler.h>
#include <fstream>

SubmitProviderIdHandler::SubmitProviderIdHandler(const std::string& route)
    : HTTPPacketProcessor(route) {
}

static std::string GetSteamApiKey() {
    std::ifstream authFile(ResourcesUtilities::GetResourcesFolder() / "../" / "auth.json");
    std::stringstream ss;
    ss << authFile.rdbuf();
    json authJson = json::parse(ss.str());
    return authJson.at("steamApiKey").get<std::string>();
}

static std::string ClientIp(const tcp::socket& sock) {
    return sock.remote_endpoint().address().to_string();
}

void SubmitProviderIdHandler::Process(const http::request<http::string_body>& req, tcp::socket& sock) {
    http::response<http::string_body> res;
    res.version(req.version());
    res.keep_alive(req.keep_alive());
    res.set(http::field::content_type, "application/json; charset=UTF-8");
    res.set(http::field::vary, "Origin");
    auto reply = [&](http::status st, std::string body) {
        res.result(st);
        res.body() = std::move(body);
        res.prepare_payload();
        http::write(sock, res);
    };
    if (req.method() != http::verb::post) {
        res.set(http::field::allow, "POST");
        return reply(http::status::method_not_allowed, R"({"error":"method not allowed"})");
    }
    if (GetSteamApiKey().empty()) {
        return reply(http::status::bad_request, R"({"error":"steamApiKey not configured"})");
    }

    const auto body = json::parse(req.body(), nullptr, false);
    if (body.is_discarded() || !body.contains("providerId") || !body.at("providerId").is_string()) {
        return reply(http::status::bad_request, R"({"error":"providerId required"})");
    }
    const std::string steam64 = body.at("providerId");
    if (steam64.empty()) {
        return reply(http::status::bad_request, R"({"error":"providerId required"})");
    }

    SteamValidator v(GetSteamApiKey());
    auto info = v.ValidateSteamId(steam64);
    if (!info) {
        return reply(http::status::bad_request, R"({"error":"invalid steam id"})");
    }

    AuthLatch::Get().Put(ClientIp(sock), steam64, /*latch timer in seconds*/ 120);
    // 120s for now until i sort the launcher out - astro
    return reply(http::status::ok, R"({"ok":true})");
}