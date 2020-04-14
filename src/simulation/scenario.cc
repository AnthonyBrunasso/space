#pragma once

#include "math/vec.h"
#include "network/network.cc"

#include "entity.cc"
#include "search.cc"
#include "tilemap.cc"

namespace simulation
{
enum ScenarioType {
  kTwoShip = 0,
  kCombatScenario,
  kSoloMission,
  kEmptyScenario,
  kMaxScenario,
};

static ScenarioType kScenario;
static uint64_t kFrame;

constexpr const char* kScenarioNames[kMaxScenario] = {
    "TwoShip",
    "Combat",
    "Solo",
    "Empty",
};
// Global enable/disable for shroud
constexpr uint64_t GAME_SHROUD = 0;

void
ScenarioSpawnRandomModule(ModuleKind kind, uint64_t ship_index, int num = 0)
{
  int made = 0;
  FOR_EACH_ENTITY(Module, module, {
    if (module->mkind != kind) continue;
    if (module->ship_index != ship_index) continue;
    if (ModuleBuilt(module)) continue;
    ModuleSetBuilt(module);
    ++made;
    if (made == num) break;
  });
}

void
ScenarioInitialize()
{
  int sid = kScenario;

  // Build ship.
  switch (sid) {
    case kCombatScenario: {
      // At least one ship
      Ship* ship = UseShip();
      ship->pod_capacity = 1;
      ship->level = 1;
      ship->type = kShipBlank;
    } break;
    case kTwoShip: {
      while (kUsedShip < kPlayerCount) {
        Ship* ship = UseShip();
        ship->transform.position = v2f(0.f, (kUsedShip - 1) * 1600.f);
        ship->pod_capacity = 1;
        ship->level = 1;
        ship->type = kShipCruiser;
      }
    } break;
    case kSoloMission: {
      // At least one ship
      Ship* ship = UseShip();
      ship->pod_capacity = 1;
      ship->level = 1;
      ship->type = kShipCruiser;
    } break;
    case kEmptyScenario: {
      // At least one ship
      Ship* ship = UseShip();
      ship->pod_capacity = 1;
      ship->level = 1;
      ship->type = kShipBlank;
    } break;
  }

  // Assign players to ships
  for (int i = 0; i < kPlayerCount; ++i) {
    auto* player = UsePlayer();
    player->ship_index = i % kUsedShip;
    player->camera.viewport.x = kNetworkState.player_info[i].window_width;
    player->camera.viewport.y = kNetworkState.player_info[i].window_height;
    player->mineral = 400;
    player->admin_menu_pos =
        v2f(player->camera.viewport.x - 900, player->camera.viewport.y);
    player->tile_menu_pos =
        v2f(player->camera.viewport.x - 600, player->camera.viewport.y);
    player->game_menu_pos = v2f(0.f, 400.f);
  }

  for (int i = 0; i < kUsedPlayer; ++i) {
    switch (sid) {
      case kEmptyScenario:
      case kCombatScenario:
        TilemapInitialize(i);
        break;
      case kSoloMission:
      case kTwoShip:
        TilemapInitialize(i);
        break;
    }
  }

  // Spawn units / modules.
  switch (sid) {
    case kCombatScenario: {
      SpawnCrew(*ShipTile(0, 5, 12), 0);
      SpawnEnemy(*ShipTile(0, 10, 12));
    } break;
    case kSoloMission: {
      SpawnCrew(*ShipTile(0, 8, 12), 0);
      SpawnEnemy(*ShipTile(0, 10, 15));
      SpawnEnemy(*ShipTile(0, 10, 20));
      SpawnEnemy(*ShipTile(0, 4, 20));
    } break;
    case kTwoShip: {
      for (int i = 0; i < kPlayerCount; ++i) {
        SpawnCrew(*ShipTile(i, 5, 23), i);
        SpawnAICrew(*ShipTile(i, 15, 15), i);
        ScenarioSpawnRandomModule(kModPower, i);
        ScenarioSpawnRandomModule(kModEngine, i, 5);
        ScenarioSpawnRandomModule(kModDoor, i, 12);
      }
    } break;
    default:
    case kEmptyScenario: {
    } break;
  }

  LOGFMT("Player count: %lu", kUsedPlayer);

  for (int i = 0; i < kUsedPlayer; ++i) {
    camera::InitialCamera(&kPlayer[i].camera);

    FOR_EACH_ENTITY(Unit, unit, {
      if (unit->player_index == i) {
        camera::Move(&kPlayer[i].camera, unit->position);
        break;
      }
    });
  }

  TilemapResetExterior();
  if (GAME_SHROUD) {
    TilemapResetShroud();
  }
}  // namespace simulation

void
ScenarioReset()
{
  kFrame = 0;
  // TODO (AN): GAME_QUEUE not in the registry
  kReadCommand = 0;
  kWriteCommand = 0;
  kAutoIncrementIdEntity = kInvalidId + 1;

  for (int i = 0; i < kUsedRegistry; ++i) {
    *kRegistry[i].memb_count = 0;
  }
  ScenarioInitialize();
}

bool
ScenarioOver()
{
  return false;
}

}  // namespace simulation
