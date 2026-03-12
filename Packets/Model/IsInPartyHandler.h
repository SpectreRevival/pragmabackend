#pragma once
#include <PacketProcessor.h>

class IsInPartyHandler : public WebsocketPacketProcessor {
  public:
    explicit IsInPartyHandler(const SpectreRpcType& rpcType);
    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};