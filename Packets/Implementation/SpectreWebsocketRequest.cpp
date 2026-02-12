#include "SpectreWebsocketRequest.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

SpectreWebsocketRequest::SpectreWebsocketRequest(SpectreWebsocket& sock, reqbuf req)
    : websocket(sock), requestBuf(std::move(req)) {
    json reqjson = json::parse(static_cast<const char*>(requestBuf.data().data()), static_cast<const char*>(requestBuf.data().data()) + requestBuf.size());
    reqJson = std::make_shared<json>(reqjson);
    try {
        requestType = SpectreRpcType(std::string(reqJson->at("type")));
    } catch (std::exception& e) {
        spdlog::warn("log type not found for " + reqJson->at("type").get<std::string>());
    }
    m_requestId = reqJson->at("requestId");
    payloadAsStr = reqJson->at("payload").dump();
}

std::shared_ptr<json> SpectreWebsocketRequest::GetPayload() const {
    return std::make_shared<json>((reqJson->at("payload")));
}

std::shared_ptr<json> SpectreWebsocketRequest::GetBaseJsonResponse() {
    json response;
    response.at("requestId") = m_requestId;
    response.at("type") = GetResponseType();
    response.at("payload") = json::object();
    return std::make_shared<json>(std::move(response));
}

void SpectreWebsocketRequest::SendEmptyResponse() {
    websocket.SendPacket(GetBaseJsonResponse());
}

std::string SpectreWebsocketRequest::GetResponseType() const {
    std::string resType = requestType.GetName();
    if (resType.size() >= 7 && resType.ends_with("Request")) {
        resType.erase(resType.size() - 7);
    }
    resType += "Response";
    return resType;
}
