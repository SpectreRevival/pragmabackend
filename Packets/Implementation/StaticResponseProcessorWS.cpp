#include <StaticResponseProcessorWS.h>
#include <nlohmann/json.hpp>

void StaticResponseProcessorWS::Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) {
    std::shared_ptr<json> res = packet.GetBaseJsonResponse();
    (*res)["payload"] = *staticRes;
    sock.SendPacket(res);
}