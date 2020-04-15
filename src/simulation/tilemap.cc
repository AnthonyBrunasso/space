#pragma once

#include <cassert>
#include <climits>
#include <cstdio>

#include "math/vec.h"

#include "ship/ship.cc"

#include "entity.cc"

namespace simulation
{
extern v3f ModuleBounds(ModuleKind mkind);             // in module.cc
void BfsTileEnable(Tile set_tile, uint64_t tile_dsq);  // in search.cc
Rectf FromShip(Tile tile);                             // in ship.cc

constexpr float kTileWidth = 25.0f;
constexpr float kTileHeight = 25.0f;

enum TileType {
  kTileOpen = 0,
  kTileBlock = 1,
  kTileModule = 2,
  kTilePower = kTileModule + kModPower,
  kTileEngine = kTileModule + kModEngine,
  kTileMine = kTileModule + kModMine,
  kTileTurret = kTileModule + kModTurret,
  kTileDoor = kTileModule + kModDoor,
};

static Tile kZeroTile;

bool
TileValid(Tile tile)
{
  return memcmp(&tile, &kZeroTile, sizeof(Tile)) != 0;
}

INLINE bool
TileEqualPosition(Tile lhs, Tile rhs)
{
  return memcmp(lhs.position, rhs.position, sizeof(Tile::position)) == 0;
}

INLINE bool
operator==(Tile lhs, Tile rhs)
{
  return TileEqualPosition(lhs, rhs);
}

INLINE bool
operator!=(Tile lhs, Tile rhs)
{
  return !TileEqualPosition(lhs, rhs);
}

INLINE void
TileClear(Tile* t, uint16_t clear)
{
  t->flags &= ~(clear);
}

INLINE void
TileSet(Tile* t, uint16_t set)
{
  t->flags |= set;
}

// Integer math to measure the distance squred to the center of a tile
bool
TileDsq(Tile lhs, Tile rhs, uint64_t tile_distance)
{
  int64_t dx = rhs.cx-lhs.cx;
  int64_t dy = rhs.cy-lhs.cy;
  const uint64_t dt = (tile_distance * 2) + 2;
  const uint64_t dsq = (dt * dt);
  uint64_t dx2 = ABS64(dx) * 2 + 1;
  uint64_t dy2 = ABS64(dy) * 2 + 1;
  return (dx2 * dx2 + dy2 * dy2 <= dsq);
}

constexpr int kMaxNeighbor = 8;
constexpr int kLastNeighbor = kMaxNeighbor - 1;
static const v2i kNeighbor[kMaxNeighbor] = {
    v2i(-1, 0), v2i(1, 0),  v2i(0, 1),  v2i(0, -1),
    v2i(1, 1),  v2i(-1, 1), v2i(1, -1), v2i(-1, -1),
};

Tile
TileNeighbor(Tile tile, uint64_t index)
{
  v2i n = kNeighbor[MOD_BUCKET(index, kMaxNeighbor)];

  tile.cx = BITRANGE_WRAP(tile.bitrange_xy, tile.cx + n.x);
  tile.cy = BITRANGE_WRAP(tile.bitrange_xy, tile.cy + n.y);

  return tile;
}

void
TilemapInitialize(uint64_t player_index)
{
  assert(player_index < kUsedPlayer);

  uint64_t ship_index = kPlayer[player_index].ship_index;
  assert(ship_index < kUsedShip);

  if (kUsedGrid <= ship_index) UseGrid();

  uint8_t* ship_design = nullptr;
  uint64_t bitrange_xy = kMapDefaultBits;
  int ship_type = kShip[ship_index].type;
  switch (ship_type) {
    case kShipShuttle:
      ship_design = &kShuttleDesign[0][0];
      bitrange_xy = kShuttleBits;
      break;
    case kShipCruiser:
      ship_design = &kCruiserDesign[0][0];
      bitrange_xy = kCruiserBits;
      break;
  }

  Tile* tile = &kGrid[ship_index].tilemap[0][0];
  for (int y = 0; y < (1 << bitrange_xy); ++y) {
    for (int x = 0; x < (1 << bitrange_xy); ++x) {
      uint8_t tile_type = ship_design ? *ship_design : kTileOpen;

      *tile = kZeroTile;
      tile->cx = x;
      tile->cy = y;
      tile->ship_index = ship_index;
      tile->bitrange_xy = bitrange_xy;
      tile->ship_deck = 0;

      switch (tile_type) {
        case kTileBlock: {
          tile->blocked = 1;
        } break;
        case kTilePower:
        case kTileEngine:
        case kTileMine:
        case kTileDoor:
        case kTileTurret: {
          Module* mod = UseEntityModule();
          mod->mkind = (ModuleKind)(tile_type - kTileModule);
          mod->bounds = ModuleBounds(mod->mkind);
          mod->ship_index = ship_index;
          mod->player_index = player_index;
          mod->position =
              v3f(0.f, 0.f, mod->bounds.z / 2.f) + FromShip(*tile).Center();
        } break;
      };

      ship_design += (ship_design != nullptr);
      ++tile;
    }
  }

  Ship* ship = &kShip[ship_index];
  ship->map = &kGrid[ship_index].tilemap[0][0];
  ship->map_width = (1 << bitrange_xy);
  ship->map_height = (1 << bitrange_xy);
}

v3f
TileAvoidWalls(Tile start)
{
  v2i avoidance = {};
  for (int i = 0; i < kMaxNeighbor; ++i) {
    Tile neighbor = TileNeighbor(start, i);
    if (neighbor.blocked) {
      avoidance -= kNeighbor[i];
    }
  }

  return v3f(avoidance.x, avoidance.y, 0.0f);
}

void
TilemapResetVisible()
{
  for (int i = 0; i < kUsedShip; ++i) {
    Ship* ship = &kShip[i];
    Tile* tile = ship->map;
    for (int j = 0; j < ship->map_height; ++j) {
      for (int k = 0; k < ship->map_width; ++k) {
        tile->visible = false;
        ++tile;
      }
    }
  }
}

void
TilemapResetExterior()
{
  for (int i = 0; i < kUsedShip; ++i) {
    Ship* ship = &kShip[i];
    Tile tile = *ship->map;
    tile.flags = 0;
    tile.exterior = 1;
    BfsTileEnable(tile, kMapMaxWidth);
  }
}

}  // namespace simulation
