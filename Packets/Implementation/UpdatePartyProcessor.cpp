#include <PartyDatabase.h>
#include <UpdatePartyProcessor.h>
#include <UpdatePartyRequest.pb.h>

UpdatePartyProcessor::UpdatePartyProcessor(SpectreRpcType rpcType)
    : WebsocketPacketProcessor(rpcType) {
}

namespace {
    void BumpPartyVersion(Party* party) {
        int64_t version = 0;

        if (!party->version().empty()) {
            try {
                version = std::stoll(party->version());
            } catch (...) {
                version = 0;
            }
        }

        party->set_version(std::to_string(version + 1));
    }
} // namespace

void UpdatePartyProcessor::Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) {
    std::unique_ptr<UpdatePartyRequest> req = packet.GetPayloadAsMessage<UpdatePartyRequest>();
    PartyResponse res = PartyDatabase::Get().GetPartyRes(req->partyid());
    Party* party = res.mutable_party();
    BroadcastPartyExtraInfo* broadcastExtra = party->mutable_extbroadcastparty();
    broadcastExtra->set_pool(req->requestext().pool());
    broadcastExtra->set_lobbymode(req->requestext().lobbymode());
    broadcastExtra->set_version(req->requestext().version());
    broadcastExtra->set_region(req->requestext().region());
    broadcastExtra->set_tag(req->requestext().tag());
    broadcastExtra->set_profile(req->requestext().profile());
    broadcastExtra->set_useteammmr(req->requestext().useteammmr());
    broadcastExtra->set_hasacceptableregion(req->requestext().acceptableregions_size() > 0);

    // i hate my life
    broadcastExtra->clear_custom();
    broadcastExtra->clear_standard();

    const bool isCustomParty = req->requestext().pool() == "custom";

    if (isCustomParty) {
        broadcastExtra->mutable_custom()->CopyFrom(req->requestext().custom());
    } else {
        for (const auto& entry : req->requestext().standard()) {
            (*broadcastExtra->mutable_standard())[entry.first] = entry.second;
        }
    }
    BumpPartyVersion(party);
    PartyDatabase::Get().SaveParty(res.party());
    sock.SendPacket(PartyDatabase::SerializePartyToString(res), packet.GetRequestId(), packet.GetResponseType());
}
