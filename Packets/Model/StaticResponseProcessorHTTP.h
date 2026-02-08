#pragma once

#include <PacketProcessor.h>

namespace http = boost::beast::http;

class StaticResponseProcessorHTTP : public HTTPPacketProcessor
{
  private:
    std::shared_ptr<json> m_res;

  public:
    StaticResponseProcessorHTTP(const std::string           &route,
                                const std::shared_ptr<json> &res)
        : HTTPPacketProcessor(route), m_res(res){};

    void Process(http::request<http::string_body> const &req,
                 tcp::socket                            &sock) override;
};