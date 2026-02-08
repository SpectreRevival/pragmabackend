#include <TestWebsocketClient.h>

#include "boost/asio/connect.hpp"
#include "nlohmann/json.hpp"
#include <google/protobuf/util/json_util.h>
#include <TestHTTPClient.h>

TestWebsocketClient::TestWebsocketClient(unsigned short port) :
io_ctx(),
work_guard(boost::asio::make_work_guard(io_ctx)),
nextRequestId(0)
{
    io_thread = std::thread([this]{
    io_ctx.run();
    });
    boost::asio::ip::tcp::resolver resolver(io_ctx);
    ws = std::make_shared<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>(io_ctx);
    ws->set_option(boost::beast::websocket::stream_base::decorator(
    [port](boost::beast::websocket::request_type& req)
    {
        req.set(boost::beast::http::field::host, "127.0.0.1:" + std::to_string(port));
        req.set(boost::beast::http::field::authorization, "Bearer eyJhbGciOiJSUzI1NiIsImtpZCI6ImQzSnRPcTZqeTNfSHF1d1Rzcnp0ODF3aDNCTGlBLTRmLXFNOG1qLTAtWVE9IiwidHlwIjoiSldUIn0.eyJiYWNrZW5kVHlwZSI6IkdBTUUiLCJkaXNjcmltaW5hdG9yIjoiNjE5NyIsImRpc3BsYXlOYW1lIjoiT2htIiwiZXhwIjoxNzY5NjIxNzU0LCJleHBpcmVzSW5NaWxsaXMiOiI4NjQwMDAwMCIsImV4dFNlc3Npb25JbmZvIjoie1wicGVybWlzc2lvbnNcIjowLFwiYWNjb3VudFRhZ3NcIjpbXCJjYW5hcnlcIl19IiwiZ2FtZVNoYXJkSWQiOiIwMDAwMDAwMC0wMDAwLTAwMDAtMDAwMC0wMDAwMDAwMDAwMDEiLCJpYXQiOjE3Njk1MzUzNTQsImlkUHJvdmlkZXIiOiJTVEVBTSIsImlzcyI6InByYWdtYSIsImp0aSI6ImVmMmY3NWZiLTA2NGItNGY3Yi05ZDc0LWNmODhjYzExMjE1MiIsInByYWdtYVBsYXllcklkIjoiNmRjOWQwNjktOTdmZS01NDQwLWJkYmItZWViZWVjMTA0N2U5IiwicHJhZ21hU29jaWFsSWQiOiI2ZGM5ZDA2OS05N2ZlLTU0NDAtYmRiYi1lZWJlZWMxMDQ3ZTkiLCJyZWZyZXNoSW5NaWxsaXMiOiIzNjIwMzAwMCIsInNlc3Npb25UeXBlIjoiUExBWUVSIiwic3ViIjoiNmRjOWQwNjktOTdmZS01NDQwLWJkYmItZWViZWVjMTA0N2U5In0.lwGR64O4ruZRmnad10daIPkGRn8kfusjX0kpz7GXLKEJg0gkjJiPVk33v4wVtLVjG0X_uU-tKpCqcw0LiFFhue9PyWANw82q_SHHA3ld5WmLoPuZG45vxhYEBm9LQ2Y2Xy3tE6-K-xoltcqMRLpfpuqhLhMvkzEjE3P5dBkM-wr2CNvfds76L4t2t-Tec-qrq1ie_64ChqP6a16UZcNJUKX7E6rwoyNl3nr9HyIVaDSLfPkMTk4t2l3BwjwQ0Qq-RJ0-_QDukT_1gHBQc_JVLn1Vg50mSvuLKk61heC7BnvwywTmUrJpxoGwbmHH_tJa0hc5mkFZrqovnUIql-ORSQ");
        req.set(boost::beast::http::field::sec_websocket_extensions, "permessage-deflate; client_max_window_bits");
    }
    ));
    auto const results = resolver.resolve("127.0.0.1", std::to_string(port));
    boost::asio::connect(ws->next_layer(), results.begin(), results.end());
    ws->handshake("127.0.0.1:" + std::to_string(port), "/");
}

std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> TestWebsocketClient::GetRawSocket()
{
    return ws;
}

boost::beast::flat_buffer TestWebsocketClient::SendPacket(const std::string& packet, const SpectreRpcType rpcType) const
{
    std::string final = "{\"requestId\":" + std::to_string(nextRequestId) + ",\"type\":\"" + rpcType.GetName() + "\",\"payload\":" + packet + "}";
    ws->write(boost::asio::buffer(final));
    boost::beast::flat_buffer buffer;
    boost::asio::steady_timer timer(ws->get_executor());

    // Timeout waiting for response after 3 seconds
    timer.expires_after(std::chrono::seconds(3));
    timer.async_wait([&](auto ec) {
        if (!ec) {
            boost::system::error_code ignore;
            ws->next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore);
            ws->next_layer().close(ignore);
        }
    });

    boost::system::error_code ec;
    ws->read(buffer, ec);

    timer.cancel();

    if (ec == boost::asio::error::operation_aborted) {
        return {};
    }
    return buffer;
}

boost::beast::flat_buffer TestWebsocketClient::SendPacket(const nlohmann::json& packet, SpectreRpcType rpcType) const
{
    std::string final = "{\"requestId\":" + std::to_string(nextRequestId) + ",\"type\":\"" + rpcType.GetName() + "\",\"payload\":" + packet.dump() + "}";
    ws->write(boost::asio::buffer(final));
    boost::beast::flat_buffer buffer;
    boost::asio::steady_timer timer(ws->get_executor());

    // Timeout waiting for response after 3 seconds
    timer.expires_after(std::chrono::seconds(3));
    timer.async_wait([&](auto ec) {
        if (!ec) {
            boost::system::error_code ignore;
            ws->next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore);
            ws->next_layer().close(ignore);
        }
    });

    boost::system::error_code ec;
    ws->read(buffer, ec);

    timer.cancel();

    if (ec == boost::asio::error::operation_aborted) {
        return {};
    }
    return buffer;
}

TestWebsocketClient::~TestWebsocketClient() {
    work_guard.reset();   // allow io_context to stop
    io_ctx.stop();        // stop any pending operations
    if (io_thread.joinable())
        io_thread.join(); // wait for background thread
}
