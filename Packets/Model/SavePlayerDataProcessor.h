#pragma once
#include <PacketProcessor.h>

class SavePlayerDataProcessor : public WebsocketPacketProcessor {
public:
	explicit SavePlayerDataProcessor(SpectreRpcType rpcType);

	void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};