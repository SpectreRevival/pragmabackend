#pragma once
#include "Regex.h"

#include <PacketProcessor.h>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

class RegexPayloadProcessorWS : public WebsocketPacketProcessor {
  private:
    std::unordered_map<regex, std::shared_ptr<json>> m_resMap;

  public:
    RegexPayloadProcessorWS(const SpectreRpcType& rpcType, const std::unordered_map<regex, std::shared_ptr<json>>& resMap)
        : WebsocketPacketProcessor(rpcType), m_resMap(resMap){};

    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};