#include <SpectreWebsocket.h>
#include <google/protobuf/util/json_util.h>
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

pbuf::util::JsonPrintOptions opts = []() {
    pbuf::util::JsonPrintOptions options;
    options.always_print_fields_with_no_presence = true;
    return options;
}();

static std::string ExtractBearer(const http::request<http::string_body>& req) {
    auto auth = req.base()[http::field::authorization];
    if (auth.empty()) return {};
    static constexpr std::string_view prefix = "Bearer ";
    const std::string s = std::string(auth);
    if (!s.starts_with(prefix)) return {};
    return s.substr(prefix.size() - 1);
}

static std::string DecodePlayerIdNoverify(const std::string& token) {
    try {
        const auto decoded = jwt::decode<jwt::traits::nlohmann_json>(token);

        if (decoded.has_payload_claim("pragmaPlayerId")) {
            return decoded.get_payload_claim("pragmaPlayerId").as_string();
        }

        try {
            return decoded.get_subject();
        } catch (...) {
            if (decoded.has_payload_claim("sub")) {
                return decoded.get_payload_claim("sub").as_string();
            }
        }
    } catch (const std::exception& e) {
        spdlog::warn("JWT decode failed: {}", e.what());
    }
    return {};
}

SpectreWebsocket::SpectreWebsocket(ws& sock, const http::request<http::string_body>& req)
    : socket(sock), curSequenceNumber(0) {
    socket.auto_fragment(false); // ideally we dont want to have to use this but we should be fine for now
    const auto bearer = ExtractBearer(req);
    const auto pid = bearer.empty() ? std::string() : DecodePlayerIdNoverify(bearer);

    if (!pid.empty()) {
        m_playerId = pid;
    } else {
        spdlog::error("no playerid ???? investigate me!");
        m_playerId = "1";
    }
};

const ws& SpectreWebsocket::GetRawSocket() const {
    return socket;
}

void SpectreWebsocket::SendPacket(const std::shared_ptr<json>& res) {
    json packet;
    packet["sequenceNumber"] = curSequenceNumber;
    packet["response"] = *res;
    curSequenceNumber++;
    socket.text(true);
    // dont pass temporary because beast will fragment it
    std::string msg = packet.dump();
    socket.write(boost::asio::buffer(msg));
}

void SpectreWebsocket::SendPacket(const pbuf::Message& payload, const std::string& resType, int requestId) {
    // you shan't comment on this cursedness
    std::string finalRes = "{\"sequenceNumber\":" + std::to_string(curSequenceNumber) + R"(,"response":{"requestId":)" + std::to_string(requestId) + R"(,"type":")" + resType + R"(","payload":)";
    std::string resComponent;
    if (!pbuf::util::MessageToJsonString(payload, &resComponent, opts).ok()) {
        spdlog::error("Failed to serialize pbuf message to string in SendPacket");
        throw;
    }
    finalRes += resComponent + "}}";
    curSequenceNumber++;
    socket.text(true);
    socket.write(boost::asio::buffer(finalRes));
}

void SpectreWebsocket::SendPacket(const std::string& resPayload, int requestId, const std::string& resType) {
    std::string finalRes = "{\"sequenceNumber\":" + std::to_string(curSequenceNumber) + R"(,"response":{"requestId":)" + std::to_string(requestId) + R"(,"type":")" + resType + R"(","payload":)";
    finalRes += resPayload + "}}";
    curSequenceNumber++;
    socket.text(true);
    socket.write(boost::asio::buffer(finalRes));
}

void SpectreWebsocket::SendNotification(const std::shared_ptr<json>& notifPayload, const SpectreRpcType& notificationType) {
    json packet;
    packet["sequenceNumber"] = curSequenceNumber;
    packet["notification"]["type"] = notificationType.GetName();
    packet["notification"]["payload"] = *notifPayload;
    curSequenceNumber++;
    socket.text(true);
    std::string msg = packet.dump();
    socket.write(boost::asio::buffer(msg));
}

void SpectreWebsocket::SendNotification(const std::string& notifPayload, const SpectreRpcType& notificationType) {
    std::string finalPayload = "{\"sequenceNumber\":" + std::to_string(curSequenceNumber) + R"(,"notification":{"type":")" + notificationType.GetName() + R"(","payload":)" + notifPayload + "}}";
    curSequenceNumber++;
    socket.text(true);
    socket.write(boost::asio::buffer(finalPayload));
}

void SpectreWebsocket::SendNotification(const pbuf::Message& notif, const SpectreRpcType& notificationType) {
    std::string finalRes = "{\"sequenceNumber\":" + std::to_string(curSequenceNumber) + R"(,"notification":"type":")" + notificationType.GetName() + R"(","payload":)";
    std::string payloadComponent;
    if (!pbuf::util::MessageToJsonString(notif, &payloadComponent, opts).ok()) {
        spdlog::error("Failed to serialize pbuf message to string in SendPacket");
        throw;
    }
    finalRes += payloadComponent + "}}";
    curSequenceNumber++;
    socket.text(true);
    socket.write(boost::asio::buffer(finalRes));
}

const std::string& SpectreWebsocket::GetPlayerId() {
    return m_playerId;
}