#include <Inventory.pb.h>
#include <PlayerDatabase.h>
#include <UpdateItemsV0Processor.h>
#include <UpdatesItemMessage.pb.h>
#include <algorithm>
#include <google/protobuf/util/json_util.h>
#include <spdlog/spdlog.h>

namespace pbu = google::protobuf::util;

UpdateItemsV0Processor::UpdateItemsV0Processor(SpectreRpcType rpcType)
    : WebsocketPacketProcessor(rpcType) {
}

static void PerformItemUpdate(InstancedItem& item, const InstancedItemUpdate& update) {
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
    res->at("payload").at("segment")["removedStackables"] = json::array();
    res->at("payload").at("segment")["removedInstanced"] = json::array();
    res->at("payload").at("segment")["previousVersion"] = std::to_string(invLevel);
    res->at("payload").at("segment")["instanced"] = json::array();
    res->at("payload").at("segment")["stackables"] = json::array();
    res->at("payload").at("segment")["version"] = playerInv->version();
    res->at("payload").at("delta")["instanced"] = json::array();
    res->at("payload").at("delta")["stackables"] = json::array();
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
        res->at("payload").at("delta").at("instanced").push_back(curDelta);
        res->at("payload").at("segment").at("instanced").push_back(finalItem);
    }
    PlayerDatabase::Get().SetField(FieldKey::PLAYER_INVENTORY, playerI.get(), sock.GetPlayerId());
    sock.SendPacket(res);
}
