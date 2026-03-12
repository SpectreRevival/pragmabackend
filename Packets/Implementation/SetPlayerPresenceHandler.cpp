#include <SetPlayerPresenceHandler.h>
#include <SetPresenceRequest.pb.h>
#include <PlayerDatabase.h>

void SetPlayerPresenceHandler::Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) {
    std::unique_ptr<SetPresenceRequest> req = packet.GetPayloadAsMessage<SetPresenceRequest>();

    std::unique_ptr<PlayerPresence> presence = PlayerDatabase::Get().GetField<PlayerPresence>(FieldKey::PLAYER_PRESENCE, sock.GetPlayerId());
    presence->set_basic_presence(req.basic_presence());
    PlayerDatabase::Get().SetField(FieldKey::PLAYER_PRESENCE, presence, sock.GetPlayerId());

    json response{};
    response["response"] = "Ok";
    sock.SendPacket(response);
}