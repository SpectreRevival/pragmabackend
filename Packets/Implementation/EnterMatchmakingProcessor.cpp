#include <EnterMatchmakingProcessor.h>
#include <EnterMatchmakingRequest.pb.h>
#include <PartyDatabase.h>

EnterMatchmakingProcessor::EnterMatchmakingProcessor(const SpectreRpcType& rpcType)
    : WebsocketPacketProcessor(rpcType) {
}

void EnterMatchmakingProcessor::Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) {
    const std::unique_ptr<EnterMatchmakingRequest> req = packet.GetPayloadAsMessage<EnterMatchmakingRequest>();
    const PartyResponse res = PartyDatabase::Get().GetPartyRes(req->partyid());
    sock.SendPacket(PartyDatabase::SerializePartyToString(res), packet.GetRequestId(), packet.GetResponseType());
}