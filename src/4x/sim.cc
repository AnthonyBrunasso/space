#pragma once

namespace fourx {

struct Player {
  std::string name;
};

struct Sim {
  std::vector<Player> players;
};

static Sim kSim;

const std::vector<Player>&
players()
{
  return kSim.players;
}

bool
PlayerJoin(const proto::PlayerJoinRequest& request)
{
  for (const auto& player : players()) {
    if (request.name() == player.name) {
      return false;
    }
  }
  Player player;
  player.name = request.name();
  kSim.players.push_back(player);
  return true;
}

}  // namespace fourx
