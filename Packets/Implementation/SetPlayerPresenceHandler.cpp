#include <SetPlayerPresenceHandler.h>
#include <SetPresenceRequest.pb.h>
#include <PlayerDatabase.h>

SetPlayerPresenceHandler::SetPlayerPresenceHandler(SpectreRpcType rpcType) : WebsocketPacketProcessor(rpcType) {

}

void SetPlayerPresenceHandler::Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) {
    std::unique_ptr<SetPresenceRequest> req = packet.GetPayloadAsMessage<SetPresenceRequest>();

    std::unique_ptr<PlayerPresence> presence = PlayerDatabase::Get().GetField<PlayerPresence>(FieldKey::PLAYER_PRESENCE, sock.GetPlayerId());
    presence->set_basicpresence(req->basicpresence());
    PlayerDatabase::Get().SetField(FieldKey::PLAYER_PRESENCE, presence.get(), sock.GetPlayerId());

    std::shared_ptr<json> response = packet.GetBaseJsonResponse();
    (*response)["payload"]["response"] = "Ok";
    sock.SendPacket(response);
}