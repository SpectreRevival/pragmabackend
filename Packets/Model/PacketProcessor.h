#pragma once
#include <SpectreRpcType.h>
#include <SpectreWebsocketRequest.h>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <string>

class HTTPPacketProcessor
{
  private:
    std::string m_route;
    inline static std::unordered_map<std::string, HTTPPacketProcessor *>
        HTTP_ROUTES = {};

  public:
    explicit HTTPPacketProcessor(std::string route) : m_route(std::move(route))
    {
        HTTP_ROUTES[m_route] = this;
    };
    virtual void Process(http::request<http::string_body> const &req,
                         tcp::socket                            &sock) = 0;
    virtual ~HTTPPacketProcessor()          = default;
    std::string &GetRoute() const
    {
        return const_cast<std::string &>(m_route);
    }
    static HTTPPacketProcessor *GetProcessorForRoute(const std::string &route)
    {
        auto it = HTTP_ROUTES.find(route);
        return it == HTTP_ROUTES.end() ? nullptr : it->second;
    }
};

class WebsocketPacketProcessor
{
  private:
    SpectreRpcType m_rpcType;
    inline static std::unordered_map<SpectreRpcType, WebsocketPacketProcessor *>
        WEBSOCKET_ROUTES = {};

  public:
    explicit WebsocketPacketProcessor(const SpectreRpcType &rpcType)
        : m_rpcType(rpcType)
    {
        WEBSOCKET_ROUTES[rpcType] = this;
    }
    virtual void Process(SpectreWebsocketRequest &packet,
                         SpectreWebsocket        &sock) = 0;
    virtual ~WebsocketPacketProcessor()          = default;
    const SpectreRpcType &GetType()
    {
        return m_rpcType;
    }
    static WebsocketPacketProcessor *
    GetProcessorForRpc(const SpectreRpcType &rpcType)
    {
        const auto it = WEBSOCKET_ROUTES.find(rpcType);
        return it == WEBSOCKET_ROUTES.end() ? nullptr : it->second;
    }
};