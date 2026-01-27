#pragma once
#include <string>

#include "boost/asio/ip/tcp.hpp"
#include "boost/beast/websocket/stream.hpp"
#include "nlohmann/json_fwd.hpp"
#include <SpectreRpcType.h>

class TestWebsocketClient
{
    std::string playerId;
    std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> ws;
    boost::asio::io_context io_ctx;
    int nextRequestId;
public:
    TestWebsocketClient(std::string playerId, unsigned short port);
    std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> GetRawSocket();
    boost::beast::flat_buffer SendPacket(std::string& packet, SpectreRpcType rpcType);
    boost::beast::flat_buffer SendPacket(nlohmann::json& packet, SpectreRpcType rpcType);
};
