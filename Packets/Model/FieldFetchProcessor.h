#pragma once
#include <FieldKey.h>
#include <PacketProcessor.h>
#include <PlayerDatabase.h>
#include <google/protobuf/util/json_util.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <sstream>
#include <type_traits>

namespace pbu = google::protobuf::util;

template <typename T>
class FieldFetchProcessor : public WebsocketPacketProcessor {
  private:
    PlayerDatabase& m_dbRef;
    const FieldKey field;

  public:
    FieldFetchProcessor(const SpectreRpcType& rpcType, const FieldKey& key, PlayerDatabase& dbRef)
        : WebsocketPacketProcessor(rpcType), m_dbRef(dbRef), field(key) {
    }

    FieldFetchProcessor(const SpectreRpcType& rpcType, const FieldKey& key)
        : FieldFetchProcessor(rpcType, key, PlayerDatabase::Get()) {
    }

    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override {
        sql::Statement query = m_dbRef.FormatStatement("SELECT {col} FROM {table} WHERE PlayerId=? LIMIT 1", field);
        query.bind(1, sock.GetPlayerId());
        std::unique_ptr<T> data = m_dbRef.GetField<T>(query, field);
        if (data == nullptr) {
            spdlog::error("No field value to return");
            throw;
        }
        sock.SendPacket(*data, packet.GetResponseType(), packet.GetRequestId());
    }
    static_assert(std::is_base_of<pbuf::Message, T>::value, "Type provided to FieldFetchProcessor must inherit from protobuf::Message");
};