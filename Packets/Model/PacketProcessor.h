#pragma once
#include <SpectreRpcType.h>
#include <SpectreWebsocketRequest.h>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <string>

class HTTPPacketProcessor {
  private:
    std::string route;
    inline static std::unordered_map<std::string, HTTPPacketProcessor*> httpRoutes = {};

  public:
    explicit HTTPPacketProcessor(std::string route)
        : route(std::move(route)) {
        httpRoutes[route] = this;
    };
    HTTPPacketProcessor(HTTPPacketProcessor& other) = delete;
    HTTPPacketProcessor(HTTPPacketProcessor&& other) = delete;
    virtual void Process(const http::request<http::string_body>& req, tcp::socket& sock) = 0;
    virtual ~HTTPPacketProcessor() = default;
    [[nodiscard]] const std::string& GetRoute() const {
        return route;
    }
    static HTTPPacketProcessor* GetProcessorForRoute(const std::string& route) {
        auto it = httpRoutes.find(route);
        return it == httpRoutes.end() ? nullptr : it->second;
    }
};

class WebsocketPacketProcessor {
  private:
    SpectreRpcType rpcType;
    inline static std::unordered_map<SpectreRpcType, WebsocketPacketProcessor*> websocketRoutes = {};

  public:
    explicit WebsocketPacketProcessor(const SpectreRpcType& rpcType)
        : rpcType(rpcType) {
        websocketRoutes[rpcType] = this;
    }
    virtual void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) = 0;
    virtual ~WebsocketPacketProcessor() = default;
    [[nodiscard]] const SpectreRpcType& GetType() const {
        return rpcType;
    }
    static WebsocketPacketProcessor* GetProcessorForRpc(const SpectreRpcType& rpcType) {
        const auto it = websocketRoutes.find(rpcType);
        return it == websocketRoutes.end() ? nullptr : it->second;
    }
};