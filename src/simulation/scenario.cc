#pragma once

#include "math/vec.h"
#include "network/network.cc"

#include "entity.cc"
#include "search.cc"

namespace simulation
{
// Do not add new fields to this struct you are not alright with being memset
// to 0xff or 0x0.
struct Scenario {
  enum Type {
    kTwoShip = 0,
    kCombatScenario,
    kSoloMission,
    kEmptyScenario,
    kMaxScenario,
  };
  Type type;

  // Feature enabled in the scenario.
  unsigned ship : 1;
  unsigned asteroid : 1;
  unsigned missile : 1;
  unsigned pod : 1;
  unsigned invasion : 1;
};

static Scenario kScenario;
static uint64_t kFrame;

constexpr const char* kScenarioNames[Scenario::kMaxScenario] = {
    "TwoShip", "Combat", "Solo", "Empty",
};

void
ScenarioSpawnRandomModule(ModuleKind kind, uint64_t ship_index, int num = 0)
{
  TilemapModify tm(ship_index);
  int made = 0;
  for (int i = 0; i < kUsedModule; ++i) {
    Module* module = &kModule[i];
    if (module->mkind != kind) continue;
    if (module->ship_index != ship_index) continue;
    if (ModuleBuilt(module)) continue;
    // TODO: These correspond now - will they always?
    ModuleSetBuilt(module);
    ++made;
    if (made == num) break;
  }
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
ScenarioInitialize(bool reset_features = true)
{
  int sid = kScenario.type;
  TilemapType tilemap_type = kTilemapEmpty;
  TilemapSet(0);

  // Build ship.
  uint64_t grid_index;
  switch (sid) {
    case Scenario::kCombatScenario: {
      grid_index = TilemapInitialize(kTilemapEmpty);
      // At least one ship
      kScenario.ship = 1;
      Ship* ship = UseShip();
      ship->grid_index = grid_index;
      ship->pod_capacity = 1;
      ship->level = 1;
    } break;
    case Scenario::kTwoShip: {
      for (int i = 0; i < kPlayerCount; ++i) {
        grid_index = TilemapInitialize(kTilemapShip);
        // At least one ship
        kScenario.ship = i;
        Ship* ship = UseShip();
        ship->grid_index = grid_index;
        ship->pod_capacity = i;
        ship->level = 1;
        kGrid[i].transform.position = v2f(0.f, i * 1600.f);
      }
    } break;
    case Scenario::kSoloMission: {
      grid_index = TilemapInitialize(kTilemapShip);
      // At least one ship
      kScenario.ship = 1;
      Ship* ship = UseShip();
      ship->grid_index = grid_index;
      ship->pod_capacity = 1;
      ship->level = 1;
    } break;
    case Scenario::kEmptyScenario: {
      grid_index = TilemapInitialize(kTilemapEmpty);
      // At least one ship
      kScenario.ship = 1;
      Ship* ship = UseShip();
      ship->grid_index = grid_index;
      ship->pod_capacity = 1;
      ship->level = 1;
    } break;
  }

  v2i crew_pos[2] = { v2i(5, 23), v2i(5, 7) };

  // Spawn units / modules.
  switch (sid) {
    case Scenario::kCombatScenario: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }
      SpawnCrew(v2i(5, 12), 0);
      SpawnEnemy(v2i(10, 12), 0);
    } break;
    case Scenario::kSoloMission: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }
      SpawnCrew(v2i(8, 12), 0);
      SpawnEnemy(v2i(10, 15), 0);
      SpawnEnemy(v2i(10, 20), 0);
      SpawnEnemy(v2i(4, 20), 0);
    } break;
    case Scenario::kTwoShip: {
      if (reset_features) {
        memset(&kScenario, 0xff, sizeof(kScenario));
        kScenario.missile = 0;
        kScenario.invasion = 0;
      }
      TilemapUnexplored(TilemapWorldCenter());
      SpawnCrew(crew_pos[0], 0);
      SpawnCrew(crew_pos[1], 1);
      ScenarioSpawnRandomModule(kModPower, 1);
      ScenarioSpawnRandomModule(kModPower, 0);
      ScenarioSpawnRandomModule(kModEngine, 1, 5);
      ScenarioSpawnRandomModule(kModEngine, 0, 5);
    } break;
    default:
    case Scenario::kEmptyScenario: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }
    } break;
  }
  // Always set unexplored/interior of ship
  TilemapUnexplored(TilemapWorldCenter());

  auto dims = window::GetWindowSize();
  for (int i = 0; i < kPlayerCount; ++i) {
    auto* player = UsePlayer();
    player->ship_index = i % kUsedShip;
    player->camera.viewport.x = kNetworkState.player_info[i].window_width;
    player->camera.viewport.y = kNetworkState.player_info[i].window_height;
    player->mineral = 400;
  }
  LOGFMT("Player count: %lu", kUsedPlayer);

  for (int i = 0; i < kUsedPlayer; ++i) {
    TilemapSet(kPlayer[i].ship_index);
    camera::InitialCamera(&kPlayer[i].camera);
    camera::Move(&kPlayer[i].camera, TilePosToWorld(crew_pos[i]));
  }
}  // namespace simulation

void
ScenarioReset(bool reset_features)
{
  kFrame = 0;
  // TODO (AN): GAME_QUEUE not in the registry
  kReadCommand = 0;
  kWriteCommand = 0;
  kAutoIncrementIdEntity = kInvalidEntity + 1;

  for (int i = 0; i < kUsedRegistry; ++i) {
    *kRegistry[i].memb_count = 0;
  }
  ScenarioInitialize(reset_features);
}

bool
ScenarioOver()
{
  return false;
}

}  // namespace simulation
