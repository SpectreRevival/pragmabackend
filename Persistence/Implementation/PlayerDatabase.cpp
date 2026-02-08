#include "PlayerDatabase.h"

#include <Inventory.pb.h>
#include <LegacyPlayerData.pb.h>
#include <OutfitLoadout.pb.h>
#include <PlayerData.pb.h>
#include <ProfileData.pb.h>
#include <ResourcesUtilities.h>
#include <WeaponLoadout.pb.h>

PlayerDatabase::PlayerDatabase(const fs::path &path)
    : Database(path, "players", "PlayerID", "TEXT")
{
    AddPrototype<Inventory>(FieldKey::PLAYER_INVENTORY,
                            ResourcesUtilities::GetResourcesFolder() /
                                "payloads" / "ws" / "game" /
                                "DefaultInventory.json");
    AddPrototype<OutfitLoadouts>(FieldKey::PLAYER_OUTFIT_LOADOUT,
                                 ResourcesUtilities::GetResourcesFolder() /
                                     "payloads" / "ws" / "game" /
                                     "DefaultOutfitLoadout.json");
    AddPrototype<WeaponLoadouts>(FieldKey::PLAYER_WEAPON_LOADOUT,
                                 ResourcesUtilities::GetResourcesFolder() /
                                     "payloads" / "ws" / "game" /
                                     "DefaultWeaponLoadout.json");
    AddPrototype<PlayerData>(FieldKey::PLAYER_DATA,
                             ResourcesUtilities::GetResourcesFolder() /
                                 "payloads" / "ws" / "game" /
                                 "DefaultPlayerData.json");
    AddPrototype<ProfileData>(FieldKey::PROFILE_DATA,
                              ResourcesUtilities::GetResourcesFolder() /
                                  "payloads" / "ws" / "game" /
                                  "DefaultProfile.json");
    AddPrototype<LegacyPlayerData>(FieldKey::PLAYER_LEGACY_DATA,
                                   ResourcesUtilities::GetResourcesFolder() /
                                       "payloads" / "ws" / "game" /
                                       "DefaultLegacyData.json");
}

PlayerDatabase PlayerDatabase::inst("playerdata.sqlite");

PlayerDatabase &PlayerDatabase::Get()
{
    return inst;
}