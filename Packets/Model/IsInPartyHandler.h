#pragma once
#include <PacketProcessor.h>

class IsInPartyHandler : public WebsocketPacketProcessor
{
public:
    IsInPartyHandler(SpectreRpcType rpcType);
    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};