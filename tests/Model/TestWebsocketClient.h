#pragma once
#include <string>

#include "boost/asio/ip/tcp.hpp"
#include "boost/beast/websocket/stream.hpp"
#include "nlohmann/json_fwd.hpp"
#include <SpectreRpcType.h>

class TestWebsocketClient
{
    boost::asio::io_context io_ctx;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard;
    std::thread io_thread;
    std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> ws;
    int nextRequestId;
public:
    explicit TestWebsocketClient(unsigned short port);
    ~TestWebsocketClient();
    std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> GetRawSocket();
    boost::beast::flat_buffer SendPacket(std::string& packet, SpectreRpcType rpcType);
    boost::beast::flat_buffer SendPacket(nlohmann::json& packet, SpectreRpcType rpcType);
};
