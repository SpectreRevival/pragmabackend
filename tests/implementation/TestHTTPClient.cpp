#include <TestHTTPClient.h>

#include <boost/asio/ip/tcp.hpp>

namespace beast = boost::beast; namespace http = beast::http; namespace net = boost::asio; using tcp = net::ip::tcp;

http::response<http::string_body> HTTPFetch(unsigned short port, std::string path, std::string packet, http::verb method)
{
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver(ioc);
    auto const results = resolver.resolve("localhost", std::to_string(port));
    beast::tcp_stream stream(ioc);
    stream.connect(results);
    http::request<http::string_body> req{
    method, path, 11
    };
    req.set(http::field::host, "localhost");
    req.body() = packet;
    req.prepare_payload();
    http::write(stream, req);
    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(stream, buffer, res);
    stream.socket().shutdown(tcp::socket::shutdown_both);
    return res;
}