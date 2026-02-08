#pragma once
#include <PacketProcessor.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

class StaticResponseProcessorWS : public WebsocketPacketProcessor {
  private:
    std::shared_ptr<json> m_res;

  public:
    StaticResponseProcessorWS(const SpectreRpcType rpcType, const std::shared_ptr<json>& res)
        : WebsocketPacketProcessor(rpcType), m_res(res){};

    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override;
};