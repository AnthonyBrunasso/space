#pragma once

#include "4x/hex_map.cc"

namespace fourx {

struct Player {
  std::string name;
  s32 id = -1;
};

struct Sim {
  std::unordered_map<s32, Player> players;
};

static Sim kSim;

const std::vector<Player>&
sim_players()
{
  return kSim.players;
}

s32
SimPlayerJoin(const proto::PlayerJoin& request)
{
  for (const auto& player : players()) {
    if (request.name() == player.name) {
      return -1;
    }
  }
  Player player;
  player.name = request.name();
  player.id = request.id();
  kSim.players[player.id] = player;
  return player.id;
}

s32
SimMapCreate()
{
}

}  // namespace fourx
