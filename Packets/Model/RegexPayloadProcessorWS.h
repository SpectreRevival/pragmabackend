#pragma once
#include "Regex.h"

#include <PacketProcessor.h>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

class RegexPayloadProcessorWS : public WebsocketPacketProcessor {
  private:
    std::unordered_map<Regex, std::shared_ptr<json>> resMap;

  public:
    RegexPayloadProcessorWS(const SpectreRpcType& rpcType, const std::unordered_map<Regex, std::shared_ptr<json>>& resMap)
        : WebsocketPacketProcessor(rpcType), resMap(resMap) {};

    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};