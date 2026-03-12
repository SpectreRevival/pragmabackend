#pragma once
#include <FieldKey.h>
#include <PacketProcessor.h>
#include <PlayerDatabase.h>
#include <spdlog/spdlog.h>
#include <type_traits>

template <typename T>
class FieldFetchProcessor : public WebsocketPacketProcessor {
  private:
    PlayerDatabase& dbRef;
    const FieldKey field;

  public:
    FieldFetchProcessor(const SpectreRpcType& rpcType, const FieldKey& key, PlayerDatabase& dbRef)
        : WebsocketPacketProcessor(rpcType), dbRef(dbRef), field(key) {
    }

    FieldFetchProcessor(const SpectreRpcType& rpcType, const FieldKey& key)
        : FieldFetchProcessor(rpcType, key, PlayerDatabase::Get()) {
    }

    void Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) override {
        sql::Statement query = dbRef.FormatStatement("SELECT {col} FROM {table} WHERE PlayerId=? LIMIT 1", field);
        query.bind(1, sock.GetPlayerId());
        std::unique_ptr<T> data = dbRef.GetField<T>(query, field);
        if (data == nullptr) {
            spdlog::error("No field value to return");
            throw;
        }
        sock.SendPacket(*data, packet.GetResponseType(), packet.GetRequestId());
    }
    static_assert(std::is_base_of<pbuf::Message, T>::value, "Type provided to FieldFetchProcessor must inherit from protobuf::Message");
};