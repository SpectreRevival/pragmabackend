#pragma once

#include <PacketProcessor.h>

namespace http = boost::beast::http;

class StaticResponseProcessorHTTP : public HTTPPacketProcessor {
  private:
    std::shared_ptr<json> staticRes;

  public:
    StaticResponseProcessorHTTP(const std::string& route, const std::shared_ptr<json>& res)
        : HTTPPacketProcessor(route), staticRes(res) {};

    void Process(const http::request<http::string_body>& req, tcp::socket& sock) override;
};