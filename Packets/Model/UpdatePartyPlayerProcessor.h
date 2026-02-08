#pragma once
#include <PacketProcessor.h>

class UpdatePartyPlayerProcessor : public WebsocketPacketProcessor
{
  public:
    explicit UpdatePartyPlayerProcessor(SpectreRpcType rpcType);
    void Process(SpectreWebsocketRequest &packet,
                 SpectreWebsocket        &sock) override;
};