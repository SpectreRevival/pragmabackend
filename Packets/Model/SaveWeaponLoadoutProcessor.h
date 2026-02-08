#pragma once
#include <PacketProcessor.h>

class SaveWeaponLoadoutProcessor : public WebsocketPacketProcessor{
public:
    explicit SaveWeaponLoadoutProcessor(SpectreRpcType rpcType);

    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};