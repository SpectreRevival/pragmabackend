#pragma once
#include <PacketProcessor.h>

class GetLoginDataProcessor : public WebsocketPacketProcessor {
  public:
    explicit GetLoginDataProcessor(const SpectreRpcType& rpcType);

    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};