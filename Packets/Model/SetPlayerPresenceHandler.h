#pragma once
#include <PacketProcessor.h>

class SetPlayerPresenceHandler : public WebsocketPacketProcessor {
  public:
    explicit SetPlayerPresenceHandler(SpectreRpcType rpcType);

    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};

static void UpdatePlayerPresence(PlayerPresence& newPresence, const std::string& playerId);