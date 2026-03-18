#include "PlayerDatabase.h"
#include "SetPlayerPresenceHandler.h"

#include <FriendsList.pb.h>
#include <GetFriendsListAndRegisterOnlineHandler.h>
#include <PlayerPresence.pb.h>

GetFriendsListAndRegisterOnlineHandler::GetFriendsListAndRegisterOnlineHandler(SpectreRpcType rpcType) : WebsocketPacketProcessor(rpcType) {

}

void GetFriendsListAndRegisterOnlineHandler::Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) {
    std::unique_ptr<FriendsList> friendsList = PlayerDatabase::Get().GetField<FriendsList>(FieldKey::FRIENDS_LIST, sock.GetPlayerId());
    std::unique_ptr<PlayerPresence> presence = PlayerDatabase::Get().GetField<PlayerPresence>(FieldKey::PLAYER_PRESENCE, sock.GetPlayerId());
    presence->set_basicpresence(Online);
    UpdatePlayerPresence(*presence, sock.GetPlayerId());
    sock.SendPacket(*friendsList, packet.GetResponseType(), packet.GetRequestId());
}