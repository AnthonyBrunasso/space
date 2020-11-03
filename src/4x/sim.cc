#pragma once

#include "4x/hex_map.cc"

namespace fourx {

struct Player {
  s32 id;
  std::string name;
};

struct Sim {
  std::unordered_map<s32, Player> players;
};

static Sim kSim;

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
  printf("[SIM] player %s joined the game\n", player.name.c_str()); 
}

void
SimMapCreate(const proto::MapCreate& map_create)
{
  printf("[SIM] create map size: %i\n", map_create.size()); 
}

void
SimStart(const proto::SimStart& sim_start)
{
  printf("[SIM] start\n"); 
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
    default: break;
  }
}

}  // namespace fourx
