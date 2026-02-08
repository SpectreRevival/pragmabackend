#include "Notification.h"
#include "Party.pb.h"
#include "SpectreRpcType.h"
#include "SpectreWebsocket.h"

#include <PartyDetailsNotification.h>
#include <string>

PartyDetailsNotification::PartyDetailsNotification(const std::string& partyId, SpectreRpcType notificationType)
    : Notification(notificationType) {
    payload = PartyDatabase::Get().GetPartyRes(partyId);
}

PartyDetailsNotification::PartyDetailsNotification(const PartyResponse& partyRes, const SpectreRpcType notificationType)
    : Notification(notificationType), payload(partyRes) {
    
}

PartyDetailsNotification::PartyDetailsNotification(const Party& party, const SpectreRpcType notificationType)
    : Notification(notificationType) {
    payload.mutable_party()->CopyFrom(party);
}

void PartyDetailsNotification::SendTo(SpectreWebsocket& sock) const {
    sock.SendNotification(PartyDatabase::SerializePartyToString(payload), GetNotificationType());
}