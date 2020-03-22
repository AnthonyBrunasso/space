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
    kGameScenario,
    kCombatGroup,
    kAI,
    kPod,
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

// Hacky game scenario stuff.
struct TwoShipScenario {
  bool enemies_spawned = false;
  uint64_t ship_two_idx;
};

static TwoShipScenario kTwoShipScenario;

constexpr const char* kScenarioNames[Scenario::kMaxScenario] = {
    "TwoShip", "Combat", "Solo", "Game", "CombatGroup", "AI", "Pod", "Empty",
};

int
AssignPlayerId()
{
  static int id = 0;
  return (id++ % kPlayerCount);
}

void
SpawnCrew(int count)
{
  v3f pos[] = {v3f(300.f, 300.f, 0.f), v3f(100.f, 130.f, 0),
               v3f(300.f, 400.f, 0), v3f(650.f, 460.f, 0),
               v3f(100.f, 577.f, 0.f)};
  const v3f scale = v3f(0.25f, 0.25f, 0.f);
  uint8_t attrib[CREWA_MAX] = {11, 10, 11, 10};
  for (int i = 0; i < ARRAY_LENGTH(pos) && i < count; ++i) {
    Unit* unit = UseIdUnit();
    unit->ship_index = 0;
    unit->transform.position = pos[i];
    unit->transform.scale = scale;
    memcpy(unit->acurrent, attrib, sizeof(attrib));
    unit->kind = kOperator;
    unit->spacesuit = 1;
    // Everybody is unique!
    unit->mskill = i;
    unit->player_id = AssignPlayerId();
  }
}

void
TilemapUnexplored(v3f world_position)
{
  Tile keep_bits;
  memset(&keep_bits, 0xff, sizeof(Tile));
  keep_bits.explored = 0;
  keep_bits.exterior = 0;
  Tile set_bits;
  memset(&set_bits, 0x00, sizeof(Tile));
  float tile_world_distance = kMapWidth * kTileWidth;
  BfsMutate(world_position, keep_bits, set_bits,
            tile_world_distance * tile_world_distance);
}

uint32_t
ScenarioSpawnEnemy(v2i tile_position, uint64_t ship_index)
{
  // Uses raii to revert ship index back to whatever was set.
  TilemapModify tm(ship_index);
  Unit* enemy = UseIdUnit();
  enemy->ship_index = ship_index;
  enemy->transform.position = TilePosToWorld(tile_position);
  enemy->transform.scale = v3f(0.25f, 0.25f, 0.f);
  enemy->alliance = kEnemy;
  enemy->kind = kAlien;
  enemy->attack_radius = 30.f;
  enemy->speed = 0.5f;
  BB_SET(enemy->bb, kUnitBehavior, kUnitBehaviorAttackWhenDiscovered);
  return enemy->id;
}

void
ScenarioSpawnCrew(v2i tile_position, uint64_t ship_index)
{
  Unit* unit = UseIdUnit();
  unit->ship_index = ship_index;
  unit->transform.position = TilePosToWorld(tile_position);
  unit->transform.scale = v3f(0.25f, 0.25f, 0.f);
  unit->kind = kOperator;
  unit->spacesuit = 1;
  unit->speed = 1.0f;
  unit->player_id = AssignPlayerId();
}

void
ScenarioInitialize(bool reset_features = true)
{
  int sid = kScenario.type;
  TilemapType tilemap_type = kTilemapEmpty;
  TilemapSet(0);
  switch (sid) {
    case Scenario::kGameScenario: {
      if (reset_features) {
        memset(&kScenario, 0xff, sizeof(kScenario));
      }
      SpawnCrew(5);
      kUnit[4].spacesuit = 1;
      tilemap_type = kTilemapShip;
    } break;
    case Scenario::kCombatScenario: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }
      UseIdUnit();
      kUnit[0].ship_index = 0;
      kUnit[0].transform.position = v3f(300.f, 300.f, 0.f);
      kUnit[0].transform.scale = v3f(0.25f, 0.25f, 0.f);
      kUnit[0].player_id = AssignPlayerId();

      UseIdUnit();
      kUnit[1].ship_index = 0;
      kUnit[1].transform.position = v3f(400.f, 300.f, 0.f);
      kUnit[1].transform.scale = v3f(0.25f, 0.25f, 0.f);
      kUnit[1].kind = kAlien;
      kUnit[1].alliance = kEnemy;

    } break;
    case Scenario::kSoloMission: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }
      // One unit
      UseIdUnit();
      kUnit[0].ship_index = 0;
      kUnit[0].transform.position = v3f(300.f, 300.f, 0.f);
      kUnit[0].transform.scale = v3f(0.25f, 0.25f, 0.f);
      kUnit[0].player_id = AssignPlayerId();

      UseIdUnit();
      kUnit[1].ship_index = 0;
      kUnit[1].transform.position = v3f(440.f, 300.f, 0.f);
      kUnit[1].transform.scale = v3f(0.25f, 0.25f, 0.f);
      kUnit[1].alliance = kEnemy;
      kUnit[1].kind = kAlien;
      kUnit[1].attack_radius = 30.f;
      kUnit[1].speed = 0.5f;
      BB_SET(kUnit[1].bb, kUnitBehavior, kUnitBehaviorAttackWhenDiscovered);

      UseIdUnit();
      kUnit[2].ship_index = 0;
      kUnit[2].transform.position = v3f(440.f, 500.f, 0.f);
      kUnit[2].transform.scale = v3f(0.25f, 0.25f, 0.f);
      kUnit[2].alliance = kEnemy;
      kUnit[2].kind = kAlien;
      kUnit[2].attack_radius = 30.f;
      kUnit[2].speed = 0.5f;
      BB_SET(kUnit[2].bb, kUnitBehavior, kUnitBehaviorAttackWhenDiscovered);

      UseIdUnit();
      kUnit[3].ship_index = 0;
      kUnit[3].transform.position = v3f(280.f, 500.f, 0.f);
      kUnit[3].transform.scale = v3f(0.25f, 0.25f, 0.f);
      kUnit[3].alliance = kEnemy;
      kUnit[3].kind = kAlien;
      kUnit[3].attack_radius = 30.f;
      kUnit[3].speed = 0.5f;
      BB_SET(kUnit[3].bb, kUnitBehavior, kUnitBehaviorAttackWhenDiscovered);

      tilemap_type = kTilemapShip;
    } break;
    case Scenario::kTwoShip: {
      if (reset_features) {
        memset(&kScenario, 0xff, sizeof(kScenario));
        kScenario.missile = 0;
        kScenario.asteroid = 0;
      }
      ScenarioSpawnCrew(v2i(5, 23), 0);
      ScenarioSpawnCrew(v2i(5, 7), 0);
      ScenarioSpawnEnemy(v2i(20, 20), 0);
      ScenarioSpawnEnemy(v2i(18, 11), 0);
      tilemap_type = kTilemapShip;
    } break;
    case Scenario::kCombatGroup: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }
      UseIdUnit();
      kUnit[0].ship_index = 0;
      kUnit[0].transform.position = v3f(300.f, 300.f, 0.f);
      kUnit[0].transform.scale = v3f(0.25f, 0.25f, 0.f);
      kUnit[0].player_id = AssignPlayerId();

      Module* mod = UseModule();
      v2i tpos;
      WorldToTilePos(kUnit[0].transform.position, &tpos);
      mod->cx = tpos.x;
      mod->cy = tpos.y;
      mod->mkind = kModBarrack;
      mod->ship_index = 0;

      UseIdUnit();
      kUnit[1].ship_index = 0;
      kUnit[1].transform.position = v3f(400.f, 300.f, 0.f);
      kUnit[1].transform.scale = v3f(0.25f, 0.25f, 0.f);
      kUnit[1].kind = kAlien;
      kUnit[1].alliance = kEnemy;

      mod = UseModule();
      WorldToTilePos(kUnit[1].transform.position, &tpos);
      mod->cx = tpos.x;
      mod->cy = tpos.y;
      mod->mkind = kModBarrack;
      mod->ship_index = 0;

    } break;
    case Scenario::kAI: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }
      UseIdUnit();
      kUnit[0].ship_index = 0;
      kUnit[0].transform.position = v3f(300.f, 300.f, 0.f);
      kUnit[0].transform.scale = v3f(0.25f, 0.25f, 0.f);

      UseIdUnit();
      kUnit[1].ship_index = 0;
      kUnit[1].transform.position = v3f(400.f, 300.f, 0.f);
      kUnit[1].transform.scale = v3f(0.25f, 0.25f, 0.f);
      kUnit[1].alliance = kEnemy;
      kUnit[1].kind = kAlien;
      kUnit[1].attack_radius = 30.f;
      kUnit[1].speed = 0.5f;
      BB_SET(kUnit[1].bb, kUnitBehavior, kUnitBehaviorSimple);
    } break;
    case Scenario::kPod: {
      if (reset_features) {
        memset(&kScenario, 0xff, sizeof(kScenario));
        kScenario.missile = 0;
        kScenario.asteroid = 0;
      }
      ScenarioSpawnCrew(v2i(5, 23), 0);
      tilemap_type = kTilemapShip;
    } break;
    default:
    case Scenario::kEmptyScenario: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }

    } break;
  }
  kScenario.type = (Scenario::Type)sid;

  uint64_t grid_index = TilemapInitialize(tilemap_type);
  // At least one ship
  kScenario.ship = 1;
  Ship* ship = UseShip();
  ship->grid_index = grid_index;
  ship->pod_capacity = 1;
  ship->level = 1;
  // TODO (AN): works for now
  assert((ship - kShip) == grid_index);
  // Global resource pool until deeper into multiplayer
  UseResource();
  // Always set unexplored/interior of ship
  TilemapUnexplored(TilemapWorldCenter());

  switch (sid) {
    case Scenario::kTwoShip: {
      // Create a second ship and tilemap
      Ship* s2 = UseShip();
      kTwoShipScenario.ship_two_idx = TilemapInitialize(tilemap_type);
      s2->grid_index = kTwoShipScenario.ship_two_idx;
      s2->level = 1;
      kGrid[kTwoShipScenario.ship_two_idx].transform.position = v2f(600.f, 800.f);
      TilemapUnexplored(TilemapWorldCenter());
    } break;
  }

  auto dims = window::GetWindowSize();
  for (int i = 0; i < kPlayerCount; ++i) {
    auto* player = UsePlayer();
    player->camera.viewport.x = kNetworkState.player_info[i].window_width;
    player->camera.viewport.y = kNetworkState.player_info[i].window_height;
  }
  LOGFMT("Player count: %lu", kUsedPlayer);

  for (int i = 0; i < kUsedPlayer; ++i) {
    camera::InitialCamera(&kPlayer[i].camera);
  }
}  // namespace simulation

void
ScenarioReset(bool reset_features)
{
  // TODO (AN): GAME_QUEUE not in the registry
  kReadCommand = 0;
  kWriteCommand = 0;
  kAutoIncrementIdUnit = kInvalidUnit + 1;

  for (int i = 0; i < kUsedRegistry; ++i) {
    *kRegistry[i].memb_count = 0;
  }
  ScenarioInitialize(reset_features);
}



void
ScenarioTwoShipUpdate()
{
  // When a unit enters the second tilemap spawn enemies in the shroud.
  if (kTwoShipScenario.enemies_spawned) return;

  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    if (TilemapWorldToGrid(unit->transform.position) ==
        kTwoShipScenario.ship_two_idx) {
      LOG("Enemies Spawning.");
      TilemapSet(kTwoShipScenario.ship_two_idx);
      ScenarioSpawnEnemy(v2i(21, 12), kTwoShipScenario.ship_two_idx);
      ScenarioSpawnEnemy(v2i(19, 20), kTwoShipScenario.ship_two_idx);
      ScenarioSpawnEnemy(v2i(9, 21), kTwoShipScenario.ship_two_idx);
      kTwoShipScenario.enemies_spawned = true;
      break;
    }
  }
}

void
ScenarioUpdate()
{
  int sid = kScenario.type;
  switch (sid) {
    case Scenario::kTwoShip: {
      ScenarioTwoShipUpdate();
    } break;
    default: break;
  }
}

bool
ScenarioOver()
{
  int sid = kScenario.type;
  switch (sid) {
    case Scenario::kGameScenario:
      if (kUsedShip == 0) {
        LOG("No ships remain.");
        return true;
      }
      if (kUsedUnit == 0) {
        LOG("The crew is dead.");
        return true;
      }
  }

  return false;
}

}  // namespace simulation
