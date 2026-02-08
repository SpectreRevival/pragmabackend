#include <IsInPartyHandler.h>
#include <IsInPartyResponse.pb.h>
#include <PartyDatabase.h>

IsInPartyHandler::IsInPartyHandler(const SpectreRpcType& rpcType)
    : WebsocketPacketProcessor(rpcType) {
}

void IsInPartyHandler::Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) {
    IsInPartyResponse res;
    sql::Statement getAllMembersStatement = PartyDatabase::Get().FormatStatement(
        "SELECT {col} FROM {table}", FieldKey::PARTY_MEMBERS);
    // This is gonna be really slow if ever used at scale, should probably refactor the party API at some point, but it works for now
    for (std::unique_ptr<PartyMembers>& party : PartyDatabase::Get().GetFields<PartyMembers>(getAllMembersStatement, FieldKey::PARTY_MEMBERS)) {
        for (int i = 0; i < party->members_size(); i++) {
            if (party->members(i).playerid() == sock.GetPlayerId()) {
                res.set_isinparty(true);
                sock.SendPacket(res, packet.GetResponseType(), packet.GetRequestId());
                return;
            }
        }
    }
    sock.SendPacket(res, packet.GetResponseType(), packet.GetRequestId());
}