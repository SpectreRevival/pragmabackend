#pragma once
#include "boost/asio/ip/tcp.hpp"
#include "boost/beast/websocket/stream.hpp"
#include "nlohmann/json_fwd.hpp"

#include <SpectreRpcType.h>
#include <string>
#include <thread>

class TestWebsocketClient {
    boost::asio::io_context ioCtx;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard;
    std::thread ioThread;
    std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> ws;
    int nextRequestId;

  public:
    explicit TestWebsocketClient(unsigned short port);
    ~TestWebsocketClient();
    std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> GetRawSocket();
    [[nodiscard]] boost::beast::flat_buffer SendPacket(const nlohmann::json& packet, SpectreRpcType rpcType) const;
};
