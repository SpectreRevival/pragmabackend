#pragma once
#include "Regex.h"

#include <PacketProcessor.h>
#include <unordered_map>

class RegexPayloadProcessorHTTP : public HTTPPacketProcessor {
  private:
    std::unordered_map<Regex, std::shared_ptr<json>> resMap;

  public:
    RegexPayloadProcessorHTTP(const std::string& route, const std::unordered_map<Regex, std::shared_ptr<json>>& resMap)
        : HTTPPacketProcessor(route), resMap(resMap){};

    void Process(const http::request<http::string_body>& req, tcp::socket& sock) override;
};