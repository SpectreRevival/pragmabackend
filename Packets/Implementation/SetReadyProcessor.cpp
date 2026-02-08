#include "CreatePartyRequest.pb.h"
#include "PacketProcessor.h"
#include "SpectreRpcType.h"
#include "SpectreWebsocket.h"
#include "SpectreWebsocketRequest.h"

#include <PartyDatabase.h>
#include <SetReadyMessage.pb.h>
#include <SetReadyProcessor.h>
#include <memory>
#include <spdlog/spdlog.h>

SetReadyProcessor::SetReadyProcessor(SpectreRpcType rpcType)
    : WebsocketPacketProcessor(rpcType) {
}

void SetReadyProcessor::Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) {
    std::unique_ptr<SetReadyMessage> readymsg = packet.GetPayloadAsMessage<SetReadyMessage>();
    PartyResponse res = PartyDatabase::Get().GetPartyRes(readymsg->partyid());
    bool playerFound = false;
    for (int i = 0; i < res.party().partymembers_size(); i++) {
        if (res.party().partymembers(i).playerid() == sock.GetPlayerId()) {
            res.mutable_party()->mutable_partymembers(i)->set_isready(readymsg->ready());
            playerFound = true;
            break;
        }
    }
    if (!playerFound) {
        spdlog::warn("Didn't find player to ready up, ignoring and returning party response\nParty ID: {}\nPlayer ID: {}", readymsg->partyid(), sock.GetPlayerId());
    }
    PartyDatabase::Get().SaveParty(res.party());
    sock.SendPacket(PartyDatabase::SerializePartyToString(res), packet.GetRequestId(), packet.GetResponseType());
}