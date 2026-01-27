#include <TestWebsocketClient.h>

#include "boost/asio/connect.hpp"
#include "nlohmann/json.hpp"
#include <google/protobuf/util/json_util.h>

TestWebsocketClient::TestWebsocketClient(std::string playerId, unsigned short port) :
io_ctx(),
nextRequestId(0)
{
    nextRequestId = 0;
    boost::asio::ip::tcp::resolver resolver(io_ctx);
    ws = std::make_shared<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>(io_ctx);
    auto const results = resolver.resolve("localhost", std::to_string(port));
    boost::asio::connect(ws->next_layer(), results.begin(), results.end());
    ws->handshake("localhost", "/");
}

std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> TestWebsocketClient::GetRawSocket()
{
    return ws;
}

boost::beast::flat_buffer TestWebsocketClient::SendPacket(std::string& packet, SpectreRpcType rpcType)
{
    std::string final = "{\"requestId\":" + std::to_string(nextRequestId) + ",\"type\":" + rpcType.GetName() + ",\"payload\":" + packet + "}";
    ws->write(boost::asio::buffer(final));
    boost::beast::flat_buffer resbuf;
    ws->read(resbuf);
    return resbuf;
}

boost::beast::flat_buffer TestWebsocketClient::SendPacket(nlohmann::json& packet, SpectreRpcType rpcType)
{
    std::string final = "{\"requestId\":" + std::to_string(nextRequestId) + ",\"type\":" + rpcType.GetName() + ",\"payload\":" + packet.dump() + "}";
    ws->write(boost::asio::buffer(final));
    boost::beast::flat_buffer resbuf;
    ws->read(resbuf);
    return resbuf;
}