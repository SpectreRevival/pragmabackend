#pragma once
#include <PacketProcessor.h>

class UpdateItemV4Processor : public WebsocketPacketProcessor {
  public:
    explicit UpdateItemV4Processor(SpectreRpcType rpcType);

    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};