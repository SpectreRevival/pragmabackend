#include <PartyDetailsNotification.h>
#include <utility>

PartyDetailsNotification::PartyDetailsNotification(const std::string& partyId, SpectreRpcType notificationType)
    : Notification(notificationType) {
    payload = PartyDatabase::Get().GetPartyRes(partyId);
}

PartyDetailsNotification::PartyDetailsNotification(PartyResponse partyRes, const SpectreRpcType notificationType)
    : Notification(notificationType), payload(std::move(partyRes)) {
}

PartyDetailsNotification::PartyDetailsNotification(const Party& party, const SpectreRpcType notificationType)
    : Notification(notificationType) {
    payload.mutable_party()->CopyFrom(party);
}

void PartyDetailsNotification::SendTo(SpectreWebsocket& sock) const {
    sock.SendNotification(PartyDatabase::SerializePartyToString(payload), GetNotificationType());
}