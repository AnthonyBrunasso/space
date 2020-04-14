#pragma once

#include <cassert>
#include <climits>
#include <cstdio>

#include "math/vec.h"

#include "ship/cruiser.cc"

#include "entity.cc"

namespace simulation
{
extern v3f ModuleBounds(ModuleKind mkind);             // in module.cc
void BfsTileEnable(Tile set_tile, uint64_t tile_dsq);  // in search.cc
Rectf FromShip(Tile tile);                             // in ship.cc

constexpr float kTileWidth = 25.0f;
constexpr float kTileHeight = 25.0f;

Grid* kCurrentGrid;

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
TileDsq(int64_t dx, int64_t dy, uint64_t tile_distance)
{
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

Tile*
TilePtr(uint16_t x, uint16_t y)
{
  if (!kCurrentGrid) return nullptr;
  if (x < 0) return nullptr;
  if (x >= kMapWidth) return nullptr;
  if (y < 0) return nullptr;
  if (y >= kMapHeight) return nullptr;

  return &kCurrentGrid->tilemap[y][x];
}

Tile*
TilePtr(Tile t)
{
  v2i grid(t.cx, t.cy);
  return &kCurrentGrid->tilemap[grid.y][grid.x];
}

Tile
TileRandom()
{
  return *TilePtr(MOD_BUCKET(rand(), kMapWidth),
                  MOD_BUCKET(rand(), kMapHeight));
}

Tile
TileNeighbor(Tile tile, uint64_t index)
{
  v2i n = kNeighbor[MOD_BUCKET(index, kMaxNeighbor)];

  tile.cx = BITRANGE_WRAP(tile.bitrange_xy, tile.cx + n.x);
  tile.cy = BITRANGE_WRAP(tile.bitrange_xy, tile.cy + n.y);

  return tile;
}

void
TilemapClear()
{
  kCurrentGrid = nullptr;
}

class TilemapModify
{
 public:
  TilemapModify(uint64_t ship_index)
  {
    prev_grid = kCurrentGrid;
    // TODO (AN): For now grid_index is ship_index
    ok = ship_index < kUsedGrid && ship_index < kUsedShip;

    if (!ok) return;

    // TODO (AN): For now grid_index is ship_index
    kCurrentGrid = &kGrid[ship_index];
  }

  ~TilemapModify()
  {
    kCurrentGrid = prev_grid;
  }

  Grid* prev_grid;
  bool ok;
};

void
TilemapInitialize(uint64_t player_index)
{
  assert(player_index < kUsedPlayer);

  uint64_t ship_index = kPlayer[player_index].ship_index;
  assert(ship_index < kUsedShip);
  Ship* ship = &kShip[ship_index];

  if (kUsedGrid <= ship_index) 
    UseGrid();

  TilemapModify tm(ship_index);
  if (ship->type == kShipBlank) {
    for (int y = 0; y < kMapHeight; ++y) {
      for (int x = 0; x < kMapWidth; ++x) {
        Tile* tile = TilePtr(x, y);
        *tile = kZeroTile;
        tile->cx = x;
        tile->cy = y;
        tile->ship_index = ship_index;
        tile->bitrange_xy = kMapBits;
      }
    }
    return;
  }

  for (int y = 0; y < kMapHeight; ++y) {
    for (int x = 0; x < kMapWidth; ++x) {
      Tile* tile = TilePtr(x, y);
      *tile = kZeroTile;
      tile->cx = x;
      tile->cy = y;
      tile->ship_index = ship_index;
      tile->bitrange_xy = kMapBits;
      tile->blocked = (kCruiserGrid[y][x] == kTileBlock);
      tile->nooxygen = 0;

      switch (kCruiserGrid[y][x]) {
        case kTilePower:
        case kTileEngine:
        case kTileMine:
        case kTileDoor:
        case kTileTurret: {
          Module* mod = UseEntityModule();
          mod->mkind = (ModuleKind)(kCruiserGrid[y][x] - kTileModule);
          mod->bounds = ModuleBounds(mod->mkind);
          mod->ship_index = ship_index;
          mod->player_index = player_index;
          mod->position =
              v3f(0.f, 0.f, mod->bounds.z / 2.f) + FromShip(*tile).Center();
        } break;
      };
    }
  }
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
TilemapResetShroud()
{
  for (int i = 0; i < kUsedShip; ++i) {
    TilemapModify tm(i);
    Tile set_tile = kZeroTile;
    set_tile.cx = kMapWidth / 2;
    set_tile.cy = kMapHeight / 2;
    set_tile.bitrange_xy = kMapBits;
    set_tile.shroud = 1;
    BfsTileEnable(set_tile, kMapWidth);
  }
}

void
TilemapResetVisible()
{
  for (int i = 0; i < kUsedShip; ++i) {
    TilemapModify tm(i);
    for (int j = 0; j < kMapHeight; ++j) {
      for (int k = 0; k < kMapWidth; ++k) {
        kCurrentGrid->tilemap[j][k].visible = false;
      }
    }
  }
}

void
TilemapResetExterior()
{
  for (int i = 0; i < kUsedShip; ++i) {
    TilemapModify tm(i);
    Tile set_tile = kZeroTile;
    set_tile.bitrange_xy = kMapBits;
    set_tile.exterior = 1;
    BfsTileEnable(set_tile, kMapWidth);
  }
}

}  // namespace simulation
