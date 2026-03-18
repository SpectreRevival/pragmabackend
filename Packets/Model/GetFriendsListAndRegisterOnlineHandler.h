#pragma once
#include <PacketProcessor.h>

class GetFriendsListAndRegisterOnlineHandler : public WebsocketPacketProcessor {
  public:
    explicit GetFriendsListAndRegisterOnlineHandler(SpectreRpcType rpcType);

    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};