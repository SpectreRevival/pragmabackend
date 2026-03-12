#include <FetchLoadoutsRequest.pb.h>
#include <FetchPlayerLoadoutsProcessor.h>
#include <PlayerDatabase.h>
#include <WeaponLoadout.pb.h>

FetchPlayerLoadoutsProcessor::FetchPlayerLoadoutsProcessor(const SpectreRpcType& rpcType)
    : WebsocketPacketProcessor(rpcType) {
}

void FetchPlayerLoadoutsProcessor::Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) {
    const std::unique_ptr<FetchLoadoutsRequest> req = packet.GetPayloadAsMessage<FetchLoadoutsRequest>();
    const std::unique_ptr<WeaponLoadouts> loadouts = PlayerDatabase::Get().GetField<WeaponLoadouts>(FieldKey::PLAYER_WEAPON_LOADOUT, req->playerid());
    FetchLoadoutsResponse res;
    for (int i = 0; i < loadouts->weaponloadoutdata_size(); i++) {
        LoadoutId* cur = res.add_weaponloadoutdata();
        cur->set_playerid(req->playerid());
        cur->set_loadoutid(loadouts->weaponloadoutdata(i).loadoutid());
    }
    sock.SendPacket(res, packet.GetResponseType(), packet.GetRequestId());
}