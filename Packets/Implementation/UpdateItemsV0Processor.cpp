#include "FieldKey.h"
#include "InstancedItem.pb.h"
#include "ItemUpdate.pb.h"
#include "PacketProcessor.h"
#include "SpectreRpcType.h"
#include "SpectreWebsocket.h"
#include "SpectreWebsocketRequest.h"

#include <Inventory.pb.h>
#include <PlayerDatabase.h>
#include <SQLiteCpp/Statement.h>
#include <UpdateItemsV0Processor.h>
#include <UpdatesItemMessage.pb.h>
#include <algorithm>
#include <cctype>
#include <google/protobuf/util/json_util.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>

namespace pbu = google::protobuf::util;

UpdateItemsV0Processor::UpdateItemsV0Processor(SpectreRpcType rpcType)
    : WebsocketPacketProcessor(rpcType) {
}

void PerformItemUpdate(InstancedItem& item, const InstancedItemUpdate& update) {
    item.mutable_ext()->set_viewed(update.ext().setviewed());
}

void UpdateItemsV0Processor::Process(SpectreWebsocketRequest& packet, SpectreWebsocket& sock) {
    std::shared_ptr<json> res = packet.GetBaseJsonResponse();
    sql::Statement invQuery = PlayerDatabase::Get().FormatStatement(
        "SELECT {col} from {table} WHERE PlayerId = ?",
        FieldKey::PLAYER_INVENTORY);
    invQuery.bind(1, sock.GetPlayerId());
    std::unique_ptr<Inventory> playerI = PlayerDatabase::Get().GetField<Inventory>(invQuery, FieldKey::PLAYER_INVENTORY);
    pbu::JsonPrintOptions opts;
    opts.always_print_fields_with_no_presence = true;
    FullInventory* playerInv = playerI->mutable_full();
    int invLevel = stoi(playerInv->version());
    playerInv->set_version(std::to_string(invLevel + 1));
    (*res)["payload"]["segment"]["removedStackables"] = json::array();
    (*res)["payload"]["segment"]["removedInstanced"] = json::array();
    (*res)["payload"]["segment"]["previousVersion"] = std::to_string(invLevel);
    (*res)["payload"]["segment"]["instanced"] = json::array();
    (*res)["payload"]["segment"]["stackables"] = json::array();
    (*res)["payload"]["segment"]["version"] = playerInv->version();
    (*res)["payload"]["delta"]["instanced"] = json::array();
    (*res)["payload"]["delta"]["stackables"] = json::array();
    std::unique_ptr<UpdatesItemMessage> itemUpdates = packet.GetPayloadAsMessage<UpdatesItemMessage>();
    pbuf::util::JsonPrintOptions options;
    options.always_print_fields_with_no_presence = true;
    for (const InstancedItemUpdate& itemUpdate : itemUpdates->instanceditemupdates()) {
        std::string instanceId = itemUpdate.instanceid();
        std::ranges::transform(instanceId, instanceId.begin(),
                               [](unsigned char c) { return std::tolower(c); });
        InstancedItem* curItem = nullptr;
        for (int i = 0; i < playerInv->instanced_size(); i++) {
            std::string testInstId = playerInv->instanced(i).instanceid();
            if (testInstId == instanceId) {
                curItem = playerInv->mutable_instanced(i);
                break;
            }
        }
        if (curItem == nullptr) {
            spdlog::warn("Couldn't find item with instance id {} in a item update request, skipping", instanceId);
            continue;
        }
        json curDelta;
        curDelta["catalogId"] = curItem->catalogid();
        curDelta["operation"] = "UPDATED";
        curDelta["tags"] = json::array();
        std::string itemInitialStr;
        if (!pbu::MessageToJsonString(*curItem, &itemInitialStr, opts).ok()) {
            spdlog::error("Failed to convert item to string");
            throw std::runtime_error("Failed to convert item to string");
        }
        json itemInitial = json::parse(itemInitialStr);
        curDelta["initial"] = itemInitial;
        PerformItemUpdate(*curItem, itemUpdate);
        std::string finalItemStr;
        if (!pbu::MessageToJsonString(*curItem, &finalItemStr, opts).ok()) {
            spdlog::error("Failed to convert item to string");
            throw std::runtime_error("Failed to convert item to string");
        }
        json finalItem = json::parse(finalItemStr);
        curDelta["final"] = finalItem;
        (*res)["payload"]["delta"]["instanced"].push_back(curDelta);
        (*res)["payload"]["segment"]["instanced"].push_back(finalItem);
    }
    PlayerDatabase::Get().SetField(FieldKey::PLAYER_INVENTORY, playerI.get(), sock.GetPlayerId());
    sock.SendPacket(res);
}
