#pragma once
#include <PacketProcessor.h>

class SubmitProviderIdHandler : public HTTPPacketProcessor {
  public:
    explicit SubmitProviderIdHandler(const std::string& route);

    void Process(const http::request<http::string_body>& req, tcp::socket& sock) override;
};