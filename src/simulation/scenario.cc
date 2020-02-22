#pragma once

#include "math/vec.h"

#include "entity.cc"
#include "search.cc"

namespace simulation
{
struct Scenario {
  enum Type {
    kGameScenario = 0,
    kCombatScenario,
    kSoloMission,
    kEmptyScenario,
    kMaxScenario,
  };
  Type type;

  // Feature enabled in the scenario.
  unsigned ai : 1;
  unsigned ship : 1;
  unsigned asteroid : 1;
  unsigned missile : 1;
  unsigned pod : 1;
  unsigned tilemap : 2;
};

static Scenario kScenario;
constexpr const char* kScenarioNames[Scenario::kMaxScenario] = {
    "Game",
    "Combat",
    "Solo",
    "Empty",
};

void
InitializeScenario(bool reset_features = true)
{
  int sid = kScenario.type;
  switch (sid) {
    case Scenario::kGameScenario: {
      if (reset_features) {
        memset(&kScenario, 0xff, sizeof(kScenario));
      }
      v3f pos[] = {v3f(300.f, 300.f, 0.f), v3f(100.f, 130.f, 0),
                   v3f(300.f, 400.f, 0), v3f(650.f, 460.f, 0),
                   v3f(100.f, 577.f, 0.f)};
      const v3f scale = v3f(0.25f, 0.25f, 0.f);
      uint8_t attrib[CREWA_MAX] = {11, 10, 11, 10};
      for (int i = 0; i < ARRAY_LENGTH(pos); ++i) {
        Unit* unit = UseIdUnit();
        unit->transform.position = pos[i];
        unit->transform.scale = scale;
        memcpy(unit->acurrent, attrib, sizeof(attrib));
        // Everybody is unique!
        unit->kind = unit->id + 1;
        printf("Create unit id %i\n", unit->id);
      }
      kUnit[4].spacesuit = 1;

      // One ship
      UseShip();
      kShip[0].running = true;
      kShip[0].level = 1;
    } break;
    case Scenario::kCombatScenario: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }
      UseIdUnit();
      kUnit[0].transform.position = v3f(300.f, 300.f, 0.f);
      kUnit[0].transform.scale = v3f(0.25f, 0.25f, 0.f);
      kUnit[0].kind = kPlayerControlled;

      UseIdUnit();
      kUnit[1].transform.position = v3f(400.f, 300.f, 0.f);
      kUnit[1].transform.scale = v3f(0.25f, 0.25f, 0.f);
      kUnit[1].kind = kEnemy;
    } break;
    case Scenario::kSoloMission: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
        kScenario.ship = 1;
        kScenario.ai = 1;
        kScenario.tilemap = 2;
      }
      // One ship
      UseShip();
      kShip[0].running = true;
      kShip[0].level = 1;
      // One unit
      UseIdUnit();
      kUnit[0].transform.position = v3f(300.f, 300.f, 0.f);
      kUnit[0].transform.scale = v3f(0.25f, 0.25f, 0.f);
      kUnit[0].kind = kPlayerControlled;
    } break;
    default:
    case Scenario::kEmptyScenario: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }
    } break;
  }
  kScenario.type = (Scenario::Type)sid;

  InitializeTilemap(kScenario.tilemap);

  switch (sid) {
    case Scenario::kSoloMission: {
      Tile keep_bits;
      memset(&keep_bits, 0xff, sizeof(Tile));
      keep_bits.explored = 0;
      Tile set_bits;
      memset(&set_bits, 0x00, sizeof(Tile));
      float tile_world_distance = kMapWidth * kTileWidth;
      BfsMutate(kUnit[0].transform.position, keep_bits, set_bits,
                tile_world_distance * tile_world_distance);
    } break;
  }
}

void
ResetScenario(bool reset_features = true)
{
  // TODO (AN): GAME_QUEUE not in the registry
  kReadCommand = 0;
  kWriteCommand = 0;

  for (int i = 0; i < kUsedRegistry; ++i) {
    *kRegistry[i].memb_count = 0;
  }
  InitializeScenario(reset_features);
}

}  // namespace simulation
