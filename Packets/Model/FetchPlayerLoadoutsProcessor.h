#pragma once
#include <PacketProcessor.h>

class FetchPlayerLoadoutsProcessor : public WebsocketPacketProcessor {
public:
    explicit FetchPlayerLoadoutsProcessor(const SpectreRpcType& rpcType);
    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};