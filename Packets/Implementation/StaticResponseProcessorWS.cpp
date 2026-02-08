#include "SpectreWebsocket.h"
#include "SpectreWebsocketRequest.h"

#include <StaticResponseProcessorWS.h>
#include <memory>
#include <nlohmann/json.hpp>

void StaticResponseProcessorWS::Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) {
    std::shared_ptr<json> res = packet.GetBaseJsonResponse();
    (*res)["payload"] = *m_res;
    sock.SendPacket(res);
}