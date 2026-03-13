#include "PlayerDatabase.h"

#include <FriendsList.pb.h>
#include <GetFriendsListAndRegisterOnlineHandler.h>
#include <PlayerPresence.pb.h>

GetFriendsListAndRegisterOnlineHandler::GetFriendsListAndRegisterOnlineHandler(SpectreRpcType rpcType) : WebsocketPacketProcessor(rpcType) {

}

void GetFriendsListAndRegisterOnlineHandler::Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) {
    std::unique_ptr<FriendsList> friendsList = PlayerDatabase::Get().GetField<FriendsList>(FieldKey::FRIENDS_LIST, sock.GetPlayerId());
    std::unique_ptr<PlayerPresence> presence = PlayerDatabase::Get().GetField<PlayerPresence>(FieldKey::PLAYER_PRESENCE, sock.GetPlayerId());
    presence->set_basicpresence(Online);
    PlayerDatabase::Get().SetField(FieldKey::PLAYER_PRESENCE, presence.get(), sock.GetPlayerId());
    sock.SendPacket(*friendsList, packet.GetResponseType(), packet.GetRequestId());
}