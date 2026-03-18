#pragma once
#include "PlayerPresence.pb.h"

#include <PacketProcessor.h>

class SetPlayerPresenceHandler : public WebsocketPacketProcessor {
  public:
    explicit SetPlayerPresenceHandler(SpectreRpcType rpcType);

    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};

void UpdatePlayerPresence(PlayerPresence& newPresence, const std::string& playerId);