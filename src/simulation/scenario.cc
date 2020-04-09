#pragma once

#include "math/vec.h"
#include "network/network.cc"

#include "entity.cc"
#include "search.cc"

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

void
ScenarioSpawnRandomModule(ModuleKind kind, uint64_t ship_index, int num = 0)
{
  TilemapModify tm(ship_index);
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
TilemapUnexplored(v3f world_position)
{
  Tile keep_bits;
  memset(&keep_bits, 0xff, sizeof(Tile));
  keep_bits.explored = 1;
  keep_bits.exterior = 0;
  Tile set_bits;
  memset(&set_bits, 0x00, sizeof(Tile));
  float tile_world_distance = kMapWidth * kTileWidth;
  BfsMutate(world_position, keep_bits, set_bits,
            tile_world_distance * tile_world_distance);
}

void
ScenarioInitialize()
{
  int sid = kScenario;
  TilemapType tilemap_type = kTilemapEmpty;
  TilemapClear();

  // Build ship.
  switch (sid) {
    case kCombatScenario: {
      // At least one ship
      Ship* ship = UseShip();
      ship->pod_capacity = 1;
      ship->level = 1;
    } break;
    case kTwoShip: {
      while (kUsedShip < kPlayerCount) {
        Ship* ship = UseShip();
        ship->pod_capacity = 1;
        ship->level = 1;
        ship->transform.position = v2f(0.f, (kUsedShip - 1) * 1600.f);
      }
    } break;
    case kSoloMission: {
      // At least one ship
      Ship* ship = UseShip();
      ship->pod_capacity = 1;
      ship->level = 1;
    } break;
    case kEmptyScenario: {
      // At least one ship
      Ship* ship = UseShip();
      ship->pod_capacity = 1;
      ship->level = 1;
    } break;
  }

  // Assign players to ships
  for (int i = 0; i < kPlayerCount; ++i) {
    auto* player = UsePlayer();
    player->ship_index = i % kUsedShip;
    player->camera.viewport.x = kNetworkState.player_info[i].window_width;
    player->camera.viewport.y = kNetworkState.player_info[i].window_height;
    player->mineral = 400;
  }

  const bool FOG_DISABLE = false;
  for (int i = 0; i < kUsedPlayer; ++i) {
    switch (sid) {
      case kEmptyScenario:
      case kCombatScenario:
        TilemapInitialize(i, kTilemapEmpty, FOG_DISABLE);
        break;
      case kSoloMission:
      case kTwoShip:
        TilemapInitialize(i, kTilemapShip, FOG_DISABLE);
        break;
    }
  }

  // Spawn units / modules.
  switch (sid) {
    case kCombatScenario: {
      SpawnCrew(v2i(5, 12), 0);
      SpawnEnemy(v2i(10, 12), 0);
    } break;
    case kSoloMission: {
      SpawnCrew(v2i(8, 12), 0);
      SpawnEnemy(v2i(10, 15), 0);
      SpawnEnemy(v2i(10, 20), 0);
      SpawnEnemy(v2i(4, 20), 0);
    } break;
    case kTwoShip: {
      for (int i = 0; i < kPlayerCount; ++i) {
        TilemapModify tm(i);
        SpawnCrew(v2i(5, 23), i, i);
        SpawnAICrew(v2i(15, 15), i, i);
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
