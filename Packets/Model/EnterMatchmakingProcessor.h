#pragma once
#include <PacketProcessor.h>

class EnterMatchmakingProcessor : public WebsocketPacketProcessor {
public:
	explicit EnterMatchmakingProcessor(const SpectreRpcType& rpcType);
	void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};