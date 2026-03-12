#include <PartyDatabase.h>
#include <google/protobuf/util/json_util.h>
#include <nlohmann/json.hpp>

using ordered_json = nlohmann::ordered_json;

static void StripEmptyModeBranches(ordered_json &root) {
    if (!root.contains("party") || !root["party"].is_object()) {
        return;
    }

    auto &party = root["party"];
    if (!party.contains("extBroadcastParty") || !party["extBroadcastParty"].is_object()) {
        return;
    }

    auto &ext = party["extBroadcastParty"];

    if (ext.contains("standard") && ext["standard"].is_object() && ext["standard"].empty()) {
        ext.erase("standard");
    }

    if (ext.contains("custom") && ext["custom"].is_object() && ext["custom"].empty()) {
        ext.erase("custom");
    }
}

PartyDatabase::PartyDatabase(fs::path path) : Database(path, "parties", "PartyID", "TEXT") {
    sql::Statement colQuery(GetRawRef(), "PRAGMA table_info(" + GetTableName() + ");");

    bool hasPartyCode    = false;
    bool hasPartyVersion = false;

    while (colQuery.executeStep()) {
        std::string colName = colQuery.getColumn(1).getText();
        if (colName == "PartyCode") {
            hasPartyCode = true;
        } else if (colName == "PartyVersion") {
            hasPartyVersion = true;
        }
    }

    if (!hasPartyCode) {
        GetRaw()->exec("ALTER TABLE " + GetTableName() + " ADD COLUMN PartyCode TEXT;");
    }

    if (!hasPartyVersion) {
        GetRaw()->exec("ALTER TABLE " + GetTableName() + " ADD COLUMN PartyVersion TEXT;");
    }

    AddPrototype<BroadcastPartyExtraInfo>(FieldKey::PARTY_EXTRA_BROADCAST_INFO);
    AddPrototype<BroadcastPrivatePartyExtraInfo>(FieldKey::PARTY_PRIVATE_EXTRA_BROADCAST_INFO);
    AddPrototype<PartyMembers>(FieldKey::PARTY_MEMBERS);
}

PartyDatabase PartyDatabase::inst("playerdata.sqlite");

PartyDatabase &PartyDatabase::Get() {
    return inst;
}

void PartyDatabase::SaveParty(const Party &party) {
    PartyMembers members;
    for (int i = 0; i < party.partymembers_size(); i++) {
        members.add_members()->CopyFrom(party.partymembers(i));
    }

    SetField(FieldKey::PARTY_MEMBERS, &members, party.partyid());
    SetField(FieldKey::PARTY_EXTRA_BROADCAST_INFO, &party.extbroadcastparty(), party.partyid());
    SetField(FieldKey::PARTY_PRIVATE_EXTRA_BROADCAST_INFO, &party.extprivateplayer(), party.partyid());

    const std::string partyVersion = party.version().empty() ? "1" : party.version();

    sql::Statement setPartyMeta(
                                GetRawRef(),
                                "INSERT INTO " + GetTableName() + "(" + GetKeyFieldName() +
                                ", PartyCode, PartyVersion) VALUES(?,?,?) "
                                "ON CONFLICT(" + GetKeyFieldName() + ") DO UPDATE SET "
                                "PartyCode = excluded.PartyCode, "
                                "PartyVersion = excluded.PartyVersion;"
                               );

    setPartyMeta.bind(1, party.partyid());
    setPartyMeta.bind(2, party.invitecode());
    setPartyMeta.bind(3, partyVersion);

    try {
        setPartyMeta.exec();
    } catch (...) {
        spdlog::error("failed to set party metadata when saving party {}", party.partyid());
        throw;
    }
}

Party PartyDatabase::GetParty(const std::string &partyId) {
    Party party;

    std::unique_ptr<PartyMembers> members = GetField<PartyMembers>(FieldKey::PARTY_MEMBERS, partyId);
    if (!members) {
        spdlog::error("failed to find members list for party {}", partyId);
        throw;
    }

    for (int i = 0; i < members->members_size(); i++) {
        party.add_partymembers()->CopyFrom(members->members(i));
    }

    std::unique_ptr<BroadcastPartyExtraInfo> broadcastExtra =
            GetField<BroadcastPartyExtraInfo>(FieldKey::PARTY_EXTRA_BROADCAST_INFO, partyId);
    if (broadcastExtra) {
        party.mutable_extbroadcastparty()->CopyFrom(*broadcastExtra);
    }

    std::unique_ptr<BroadcastPrivatePartyExtraInfo> privateExtra =
            GetField<BroadcastPrivatePartyExtraInfo>(FieldKey::PARTY_PRIVATE_EXTRA_BROADCAST_INFO, partyId);
    if (privateExtra) {
        party.mutable_extprivateplayer()->CopyFrom(*privateExtra);
    }

    party.set_partyid(partyId);
    sql::Statement getPartyMeta(
                                GetRawRef(),
                                "SELECT PartyCode, PartyVersion FROM " + GetTableName() + " WHERE PartyID = ?"
                               );
    getPartyMeta.bind(1, partyId);
    if (!getPartyMeta.executeStep()) {
        spdlog::error("failed to find party metadata for party: {}", partyId);
        throw;
    }
    party.set_invitecode(getPartyMeta.getColumn("PartyCode").getString());
    party.add_preferredgameserverzones("uscentral-1");
    if (!getPartyMeta.getColumn("PartyVersion").isNull()) {
        std::string version = getPartyMeta.getColumn("PartyVersion").getString();
        party.set_version(version.empty() ? "1" : version);
    } else {
        party.set_version("1");
    }
    return party;
}

PartyResponse PartyDatabase::GetPartyRes(const std::string &partyId) {
    PartyResponse res;
    Party         party = GetParty(partyId);
    res.mutable_party()->CopyFrom(party);
    return res;
}

Party PartyDatabase::GetPartyByInviteCode(const std::string &inviteCode) {
    Party          party;
    sql::Statement getPartyMeta(
                                GetRawRef(),
                                "SELECT PartyID, PartyVersion FROM " + GetTableName() + " WHERE PartyCode = ?"
                               );
    getPartyMeta.bind(1, inviteCode);
    if (!getPartyMeta.executeStep()) {
        spdlog::error("failed to find party id for party from invite code: {}\n are you sure that this invite code hasn't expired?",
                      inviteCode);
        throw;
    }
    std::string                   partyId = getPartyMeta.getColumn("PartyID").getString();
    std::unique_ptr<PartyMembers> members = GetField<PartyMembers>(FieldKey::PARTY_MEMBERS, partyId);
    if (!members) {
        spdlog::error("failed to find members list for party {}", partyId);
        throw;
    }

    for (int i = 0; i < members->members_size(); i++) {
        party.add_partymembers()->CopyFrom(members->members(i));
    }

    std::unique_ptr<BroadcastPartyExtraInfo> broadcastExtra =
            GetField<BroadcastPartyExtraInfo>(FieldKey::PARTY_EXTRA_BROADCAST_INFO, partyId);
    if (broadcastExtra) {
        party.mutable_extbroadcastparty()->CopyFrom(*broadcastExtra);
    }

    std::unique_ptr<BroadcastPrivatePartyExtraInfo> privateExtra =
            GetField<BroadcastPrivatePartyExtraInfo>(FieldKey::PARTY_PRIVATE_EXTRA_BROADCAST_INFO, partyId);
    if (privateExtra) {
        party.mutable_extprivateplayer()->CopyFrom(*privateExtra);
    }

    party.set_partyid(partyId);
    party.set_invitecode(inviteCode);
    party.add_preferredgameserverzones("uscentral-1");

    if (!getPartyMeta.getColumn("PartyVersion").isNull()) {
        std::string version = getPartyMeta.getColumn("PartyVersion").getString();
        party.set_version(version.empty() ? "1" : version);
    } else {
        party.set_version("1");
    }

    return party;
}

PartyResponse PartyDatabase::GetPartyResByInviteCode(const std::string &inviteCode) {
    PartyResponse res;
    Party         party = GetPartyByInviteCode(inviteCode);
    res.mutable_party()->CopyFrom(party);
    return res;
}

static const std::string sharedClientDataStart = "sharedClientData\":";

std::string PartyDatabase::SerializePartyToString(const PartyResponse &partyRes) {
    std::string jsoninit;

    pbuf::util::JsonPrintOptions popts;
    popts.always_print_fields_with_no_presence = true;
    auto status                                = pbuf::util::MessageToJsonString(partyRes, &jsoninit, popts);
    if (!status.ok()) {
        spdlog::error("Failed to serialize CreatePartyProcessor response: {}", status.message());
        throw;
    }

    ordered_json parsed;
    try {
        parsed = ordered_json::parse(jsoninit);
        StripEmptyModeBranches(parsed);
        jsoninit = parsed.dump();
    } catch (const std::exception &e) {
        spdlog::error("failed to post-process party json: {}", e.what());
        throw;
    }

    std::string jsonfinal;
    size_t      pos = jsoninit.find(sharedClientDataStart);
    if (pos == std::string::npos) {
        spdlog::error("did not find sharedClientData property in CreatePartyProcessor res json, something weird has happened");
        throw;
    }

    pos       += sharedClientDataStart.size();
    jsonfinal = std::string(jsoninit.begin(), jsoninit.begin() + pos);
    jsonfinal += '\"';

    char curChar = jsoninit[pos];
    while (curChar != '}') {
        if (curChar == '\"') {
            jsonfinal += "\\\"";
        } else {
            jsonfinal += curChar;
        }
        pos++;
        curChar = jsoninit[pos];
    }

    jsonfinal += "}\"";
    jsonfinal += std::string(jsoninit.begin() + pos + 1, jsoninit.end());
    return jsonfinal;
}
