#include <GameDataStore.h>
#include <ResourcesUtilities.h>
#include <fstream>
#include <google/protobuf/util/json_util.h>
#include <spdlog/spdlog.h>
#include <sstream>

namespace pbu = google::protobuf::util;

GameDataStore GameDataStore::inst((ResourcesUtilities::GetResourcesFolder() / "payloads" / "ws" / "game" / "DefaultInventoryStore.json").string());

static std::string InventoryStoreToPayload(const InventoryContent* invStore) { // NOLINT(readability-function-cognitive-complexity)
    std::string jsonstr;
    std::string jsonstr2;
    pbu::JsonPrintOptions popts;
    popts.always_print_fields_with_no_presence = true;
    const auto status = pbu::MessageToJsonString(*invStore, &jsonstr, popts);
    if (!status.ok()) {
        spdlog::error("Failed to serialize InventoryContent to json string: {}", status.message());
        throw;
    }
    size_t curPos = 0;
    curPos = jsonstr.find("\"contentId\":", curPos);
    size_t lastPos = 0;

    while (curPos != std::string::npos) {
        curPos--;
        char curChar = jsonstr[curPos];
        jsonstr2 += std::string(jsonstr.begin() + lastPos, jsonstr.begin() + curPos);
        jsonstr2 += '\"';
        while (curChar != '}') {
            if (curChar == '\"') {
                jsonstr2 += "\\\"";
            } else {
                jsonstr2 += curChar;
            }
            curPos++;
            curChar = jsonstr[curPos];
        }
        jsonstr2 += curChar;
        jsonstr2 += '\"';
        curPos++;
        lastPos = curPos;
        curPos = jsonstr.find("\"contentId\":", curPos);
    }
    jsonstr2 += std::string(jsonstr.begin() + lastPos, jsonstr.end());
    curPos = jsonstr2.find("\"gameData\":{}");
    while (curPos != std::string::npos) {
        curPos += 11;
        jsonstr2[curPos] = '\"';
        jsonstr2[curPos + 1] = '\"';
        curPos = jsonstr2.find("\"gameData\":{}");
    }
    curPos = jsonstr2.find(R"("gameData":"{\"contentId\":\"\")");
    while (curPos != std::string::npos) {
        size_t colon = jsonstr2.find(':', curPos);
        if (colon == std::string::npos) break;
        size_t startQuote = jsonstr2.find('\"', colon + 1);
        if (startQuote == std::string::npos) break;
        size_t i = startQuote + 1;
        bool esc = false;
        for (; i < jsonstr2.size(); ++i) {
            char ch = jsonstr2[i];
            if (esc) {
                esc = false;
                continue;
            }
            if (ch == '\\') {
                esc = true;
                continue;
            }
            if (ch == '\"') break;
        }
        if (i >= jsonstr2.size()) break;
        jsonstr2.replace(startQuote, i - startQuote + 1, "{}");
        curPos = jsonstr2.find(R"("gameData":"{\"contentId\":\"\")", startQuote + 2);
    }
    return jsonstr2;
}

void GameDataStore::RefreshInventoryStoreCache(const InventoryContent* invStore) {
    inventoryStore_bufCache = InventoryStoreToPayload(invStore);
    inventoryStoreLock.unlock();
}

GameDataStore::GameDataStore(const std::string& inventoryStorePath) {
    std::ifstream invStoreFile(inventoryStorePath);
    if (!invStoreFile.is_open()) {
        spdlog::error("failed to open InventoryStore file");
        throw;
    }
    std::stringstream ss;
    ss << invStoreFile.rdbuf();
    auto status = pbu::JsonStringToMessage(ss.str(), &inventoryStore);
    if (!status.ok()) {
        spdlog::error("failed to initialize InventoryStore: {}", status.message());
        throw;
    }
    inventoryStore_bufCache = InventoryStoreToPayload(&inventoryStore);
}

std::unique_ptr<InventoryContent, std::function<void(const InventoryContent*)>> GameDataStore::InventoryStore_mut() {
    // When the returned pointer goes out of scope, call the RefreshInventoryStoreCache method in the class
    while (!inventoryStoreLock.try_lock()) {}
    return std::unique_ptr<InventoryContent, std::function<void(const InventoryContent*)>>(
        &inventoryStore,
        // Turns class method into static lambda
        [this](auto&& pH1) { RefreshInventoryStoreCache(std::forward<decltype(pH1)>(pH1)); });
}

std::unique_ptr<const InventoryContent, std::function<void(const InventoryContent*)>> GameDataStore::InventoryStore() {
    while (!inventoryStoreLock.try_lock()) {}
    return std::unique_ptr<const InventoryContent, std::function<void(const InventoryContent*)>>(
        &inventoryStore,
        [this](auto&& pH1) { UnlockInventoryStore(std::forward<decltype(pH1)>(pH1)); });
}

std::unique_ptr<const std::string, std::function<void(const std::string*)>> GameDataStore::InventoryStore_buf() {
    while (!inventoryStoreLock.try_lock()) {}
    return std::unique_ptr<std::string, std::function<void(const std::string*)>>(
        &inventoryStore_bufCache,
        [this](auto&& pH1) { UnlockInventoryStore2(std::forward<decltype(pH1)>(pH1)); });
}

GameDataStore& GameDataStore::Get() {
    return inst;
}

void GameDataStore::UnlockInventoryStore2(const std::string* /*unused*/) {
    inventoryStoreLock.unlock();
}

void GameDataStore::UnlockInventoryStore(const InventoryContent* /*unused*/) {
    inventoryStoreLock.unlock();
}