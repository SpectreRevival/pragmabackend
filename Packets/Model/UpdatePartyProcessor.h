#pragma once
#include <PacketProcessor.h>

class UpdatePartyProcessor : public WebsocketPacketProcessor {
  public:
    explicit UpdatePartyProcessor(SpectreRpcType rpcType);
    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};