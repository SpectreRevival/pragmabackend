#pragma once
#include <SpectreRpcType.h>
#include <SpectreWebsocket.h>
#include <boost/beast/core.hpp>
#include <google/protobuf/util/json_util.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace pbuf = google::protobuf;
using reqbuf   = boost::beast::flat_buffer;

class SpectreWebsocketRequest
{
  private:
    SpectreWebsocket     &m_websocket;
    reqbuf                m_requestbuf;
    SpectreRpcType        m_requestType;
    std::shared_ptr<json> m_reqjson;
    std::string           m_payloadAsStr;
    int                   m_requestId;

  public:
    SpectreWebsocketRequest(SpectreWebsocket &sock, reqbuf req);

    std::shared_ptr<json> GetPayload() const;

    template <typename T> std::unique_ptr<T> GetPayloadAsMessage()
    {
        static_assert(std::is_base_of_v<pbuf::Message, T>,
                      "Type passed to GetPayloadAsMessage must be subclass of "
                      "pbuf::Message");
        T    message;
        auto status = pbuf::util::JsonStringToMessage(m_payloadAsStr, &message);
        if (!status.ok())
        {
            spdlog::error("Failed to parse incoming request to message: {}",
                          status.message());
            throw;
        }
        return std::make_unique<T>(message);
    }

    reqbuf *GetRawBuffer()
    {
        return &m_requestbuf;
    }

    [[nodiscard]] SpectreWebsocket &GetSocket() const
    {
        return m_websocket;
    }

    [[nodiscard]] SpectreRpcType GetRequestType() const
    {
        return m_requestType;
    }

    std::string GetResponseType() const;

    [[nodiscard]] int GetRequestId() const
    {
        return m_requestId;
    }

    std::shared_ptr<json> GetBaseJsonResponse();
    void                  SendEmptyResponse();
};