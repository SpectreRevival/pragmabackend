#include <GetFriendsListAndRegisterOnlineHandler.h>

void GetFriendsListAndRegisterOnlineHandler::Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) {
    std::unique_ptr<FriendsList> friendsList = PlayerDatabase::Get().GetField<FriendsList>(FieldKey::FRIENDS_LIST, sock.GetPlayerId());
    std::unique_ptr<PlayerPresence> presence = PlayerDatabase::Get().GetField<FriendsList>(FieldKey::PLAYER_PRESENCE, sock.GetPlayerId());
    presence.set_basic_presence(PlayerBasicPresence::ONLINE);
    sock.SendPacket(friendsList);
}