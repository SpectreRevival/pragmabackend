#pragma once
#include <PacketProcessor.h>

class FetchPlayerLoadoutsProcessor : public WebsocketPacketProcessor {
public:
    explicit FetchPlayerLoadoutsProcessor(SpectreRpcType rpcType);
    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};