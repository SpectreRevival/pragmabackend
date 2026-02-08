#pragma once
#include <PacketProcessor.h>

class HeartbeatProcessor : public WebsocketPacketProcessor {
  public:
    explicit HeartbeatProcessor(const SpectreRpcType& rpcType)
        : WebsocketPacketProcessor(rpcType){};
    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override {
        packet.SendEmptyResponse();
    }
};