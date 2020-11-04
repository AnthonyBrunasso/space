#pragma once

#include "4x/hex_map.cc"

namespace fourx {

struct Player {
  s32 id;
  std::string name;
};

struct Unit {
  s32 id;
  s32 player_id;
  v2i grid_pos;
};

struct Sim {
  std::unordered_map<s32, Player> players;
  std::vector<Unit> units;
  std::unique_ptr<HexMap> hex_map;
  b8 is_game_started = false;
};

static Sim kSim;

std::vector<const Player*>
SimGetPlayers()
{
  std::vector<const Player*> players;
  for (const auto& player : kSim.players) {
    players.push_back(&player.second);
  }
  return players;
}

const std::vector<Unit>&
SimGetUnits()
{
  return kSim.units;
}

HexMap*
SimGetMap()
{
  return kSim.hex_map.get();
}

b8
SimIsGameStarted()
{
  return kSim.is_game_started;
}

Player*
SimPlayer(s32 id)
{
  auto found = kSim.players.find(id);
  if (found == kSim.players.end()) return nullptr;
  return &found->second;
}

void
SimPlayerJoin(const proto::PlayerJoin& player_join)
{
  if (Player* found_player = SimPlayer(player_join.id())) {
    printf("Player [%i] already exists as %s\n",
           found_player->id, found_player->name.c_str());
    return;
  }
  Player player;
  player.id = player_join.id();
  player.name = player_join.name();
  kSim.players[player.id] = player;
  printf("[SIM] player %s joined the game\n", player.name.c_str()); 
}

void
SimMapCreate(const proto::MapCreate& map_create)
{
  kSim.hex_map = std::make_unique<HexMap>(map_create.size());
  printf("[SIM] map created size %i\n", map_create.size());
}

void
SimStart(const proto::SimStart& sim_start)
{
  kSim.is_game_started = true;
  printf("[SIM] start\n"); 
}

void
SimUnitCreate(const proto::UnitCreate& unit_create)
{
  printf("[SIM] unit create: %s\n", unit_create.DebugString().c_str());
  static s32 kUnitId = 1;
  Unit unit;
  unit.id = kUnitId++;
  unit.player_id = unit_create.player_id();
  unit.grid_pos = v2i(unit_create.grid_x(), unit_create.grid_y());
  kSim.units.push_back(unit);
}

void
SimExecute(const proto::SimulationStepRequest& request)
{
  switch (request.step_case()) {
    case proto::SimulationStepRequest::kPlayerJoin: {
      SimPlayerJoin(request.player_join());
    } break;
    case proto::SimulationStepRequest::kMapCreate: {
      SimMapCreate(request.map_create());
    } break;
    case proto::SimulationStepRequest::kSimStart: {
      SimStart(request.sim_start());
    } break;
    case proto::SimulationStepRequest::kUnitCreate: {
      SimUnitCreate(request.unit_create());
    } break;
    default: break;
  }
}

}  // namespace fourx
