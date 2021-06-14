#pragma once

#include "4x/hex_map.cc"
#include "4x/log.cc"

namespace fourx {

struct Player {
  s32 id;
  std::string name;
};

constexpr s32 kInvalidUnit = 0;
constexpr s32 kInvalidCity = 0;

struct Unit {
  s32 id = kInvalidUnit;
  s32 player_id;
  v2i grid_pos;
  s32 action_points;
};

struct City {
  s32 id = kInvalidCity;
  s32 player_id;
  v2i grid_pos;
};

struct Sim {
  std::unordered_map<s32, Player> players;
  std::vector<Unit> units;
  std::vector<City> cities;
  std::unique_ptr<HexMap> hex_map;
  s32 active_player_id = 0;
  s32 turn = 0;
  b8 is_game_started = false;
};

static Sim kSim;

void SimExecute(const proto::SimulationStepRequest& request);

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

const std::vector<City>&
SimGetCities()
{
  return kSim.cities;
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

Unit*
SimUnit(s32 id)
{
  for (Unit& unit : kSim.units) {
    if (unit.id == id) {
      return &unit;
    }
  }
  return nullptr;
}

City*
SimCity(s32 id)
{
  for (City& city : kSim.cities) {
    if (city.id == id) {
      return &city;
    }
  }
  return nullptr;
}

Player*
SimActivePlayer()
{
  if (!kSim.active_player_id) return nullptr;
  return SimPlayer(kSim.active_player_id);
}

void
SimAllPlayersDone()
{
  printf("[SIM] all players done\n");
  for (Unit& unit : kSim.units) {
    unit.action_points = 1;
  }
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
  kSim.active_player_id = 1;
  printf("[SIM] start active player is %s\n",
         SimPlayer(kSim.active_player_id)->name.c_str()); 
}

void
SimUnitCreate(const proto::UnitCreate& unit_create)
{
  static s32 kUnitId = 1;
  Unit unit;
  unit.id = kUnitId++;
  unit.player_id = unit_create.player_id();
  unit.grid_pos = v2i(unit_create.grid_x(), unit_create.grid_y());
  unit.action_points = 1;
  kSim.units.push_back(unit);
}

void
SimUnitMove(const proto::UnitMove& unit_move)
{
  Unit* unit = SimUnit(unit_move.unit_id());
  if (!unit) {
    printf("[SIM ERROR] unit does not exist.\n");
    return;
  }
  unit->grid_pos = v2i(unit_move.to_grid_x(), unit_move.to_grid_y());
  unit->action_points = 0;
}

void
SimUnitDestroy(const proto::UnitDestroy& unit_destroy)
{
  Unit* unit = SimUnit(unit_destroy.unit_id());
  if (!unit) {
    printf("[SIM ERROR] unit does not exist.\n");
    return;
  }
  s32 destroy_i = -1;
  for (s32 i = 0; i < kSim.units.size(); ++i) {
    if (kSim.units[i].id == unit_destroy.unit_id()) {
      destroy_i = i;
      break;
    }
  }
  if (destroy_i == -1) {
    printf("[SIM ERROR] unit with id %i does not exist.\n", unit_destroy.unit_id());
    return;
  }
  kSim.units.erase(kSim.units.begin() + destroy_i);
}

void
SimCityCreate(const proto::CityCreate& city_create)
{
  static s32 kCityId = 1;
  s32 unit_on_grid = kInvalidUnit;
  // To create a city there must be a unit on this tile.
  for (const auto& unit : kSim.units) {
    if (unit.grid_pos.x == city_create.grid_x() &&
        unit.grid_pos.y == city_create.grid_y() &&
        unit.player_id == city_create.player_id()) {
      unit_on_grid = unit.id;
      break;
    }
  }

  if (unit_on_grid == kInvalidUnit) {
    printf("[SIM ERROR] unit not found on grid position %i %i\n",
           city_create.grid_x(), city_create.grid_y());
    return;
  }

  // Destroy the unit on the tile of this city.
  proto::SimulationStepRequest step_request;
  step_request.set_player_id(kSim.active_player_id);
  proto::UnitDestroy* unit_destroy = step_request.mutable_unit_destroy();
  unit_destroy->set_unit_id(unit_on_grid);
  SimExecute(step_request);

  City city;
  city.id = kCityId++;
  city.player_id = city_create.player_id();
  city.grid_pos = v2i(city_create.grid_x(), city_create.grid_y());
  kSim.cities.push_back(city);
}

bool
SimIsPlayerDone()
{
  if (!kSim.is_game_started) return false;
  Player* player = SimPlayer(kSim.active_player_id);
  assert(player != nullptr); // Should never happen?
  if (!player) return false;
  for (const Unit& unit : kSim.units) {
    if (unit.player_id != player->id) continue;
    if (unit.action_points != 0) return false;
  }
  return true;
}

void
SimExecute(const proto::SimulationStepRequest& request)
{
  printf("[SIM] %s: %s\n", request.GetTypeName().c_str(),
         request.DebugString().c_str());
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
    case proto::SimulationStepRequest::kUnitDestroy: {
      SimUnitDestroy(request.unit_destroy());
    } break;
    case proto::SimulationStepRequest::kUnitMove: {
      if (request.player_id() != kSim.active_player_id) {
        printf("[SIM ERROR] unit tried to move out of turn.\n");
        return;
      }
      SimUnitMove(request.unit_move());
    } break;
    case proto::SimulationStepRequest::kCityCreate: {
      if (request.player_id() != kSim.active_player_id) {
        printf("[SIM ERROR] player tried to create city out of turn. [active: %i id: %i]\n",
               kSim.active_player_id, request.player_id());
        return;
      }
      SimCityCreate(request.city_create());
    } break;
    default: break;
  }

  if (SimIsPlayerDone()) {
    printf("[SIM] Player %i end\n", kSim.active_player_id);
    ++kSim.active_player_id;
    if (kSim.active_player_id > kSim.players.size()) {
      ++kSim.turn;
      kSim.active_player_id = 1;
      SimAllPlayersDone();
    }
    printf("[SIM] Player %i start\n", kSim.active_player_id);
  }
}

}  // namespace fourx
