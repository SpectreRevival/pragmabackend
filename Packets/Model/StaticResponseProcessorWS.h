#pragma once
#include <PacketProcessor.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

class StaticResponseProcessorWS : public WebsocketPacketProcessor {
  private:
    std::shared_ptr<json> staticRes;

  public:
    StaticResponseProcessorWS(const SpectreRpcType rpcType, const std::shared_ptr<json>& res)
        : WebsocketPacketProcessor(rpcType), staticRes(res){};

    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};