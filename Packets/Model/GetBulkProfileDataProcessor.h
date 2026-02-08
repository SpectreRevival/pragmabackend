#pragma once
#include <PacketProcessor.h>

class GetBulkProfileDataProcessor : public WebsocketPacketProcessor {
public:
	explicit GetBulkProfileDataProcessor(const SpectreRpcType& rpcType);

	void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};