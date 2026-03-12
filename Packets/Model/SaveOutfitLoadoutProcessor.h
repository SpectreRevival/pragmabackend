#pragma once
#include <PacketProcessor.h>

class SaveOutfitLoadoutProcessor : public WebsocketPacketProcessor {
  public:
    explicit SaveOutfitLoadoutProcessor(SpectreRpcType rpcType);

    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};