#pragma once

#include <cassert>
#include <climits>
#include <cstdio>

#include "math/vec.h"

#include "entity.cc"

namespace simulation
{
constexpr float kTileWidth = 25.0f;
constexpr float kInverseTileWidth = 1.0f / kTileWidth;
constexpr float kTileHeight = 25.0f;
constexpr float kInverseTileHeight = 1.0f / kTileHeight;

Grid* kCurrentGrid;
v2f kTilemapWorldOffset;

enum TileType {
  kTileOpen = 0,
  kTileBlock = 1,
  kTileModule = 2,
  kTilePower = kTileModule + kModPower,
  kTileEngine = kTileModule + kModEngine,
  kTileMine = kTileModule + kModMine,
  kTileTurret = kTileModule + kModTurret,
  kTileVacuum = 6,
  kTileConsumable = 7,
};

enum TilemapType {
  kTilemapEmpty,
  kTilemapShip
};

static_assert(sizeof(Tile) == sizeof(uint32_t),
              "Sync TileAND with the new Tile size");
INLINE Tile
TileAND(Tile lhs, Tile rhs)
{
  uint32_t res = *(uint32_t*)&lhs & *(uint32_t*)&rhs;
  return *(Tile*)&res;
}

INLINE Tile
TileOR(Tile lhs, Tile rhs)
{
  uint32_t res = *(uint32_t*)&lhs | *(uint32_t*)&rhs;
  return *(Tile*)&res;
}

#define INVALID_TILE v2i{0, 0};

constexpr int kMaxNeighbor = 8;
constexpr int kLastNeighbor = kMaxNeighbor - 1;
static const v2i kNeighbor[kMaxNeighbor] = {
    v2i(-1, 0), v2i(1, 0),  v2i(0, 1),  v2i(0, -1),
    v2i(1, 1),  v2i(-1, 1), v2i(1, -1), v2i(-1, -1),
};

// Returns the minimum position of the tile.
v2f
TilePosToWorldMin(const v2i pos)
{
  return {kTilemapWorldOffset.x + ((float)pos.x * kTileWidth),
          kTilemapWorldOffset.y + ((float)pos.y * kTileHeight)};
}

// Returns the centered position of the tile.
v2f
TilePosToWorld(const v2i pos)
{
  return TilePosToWorldMin(pos) + v2f(kTileWidth * .5f, kTileHeight * .5f);
}

// Returns the minimum position of the tile.
v2f
TileToWorld(const Tile tile)
{
  return TilePosToWorld(v2i(tile.cx, tile.cy));
}

// Returns true for positions that exist as a tile in kTilemap
bool
TileOk(v2i pos)
{
  if (pos.x < 0) return false;
  if (pos.x >= kMapWidth) return false;
  if (pos.y < 0) return false;
  if (pos.y >= kMapHeight) return false;
  return true;
}

Tile*
TilePtr(const v2i& pos)
{
  if (!kCurrentGrid) return nullptr;
  if (!TileOk(pos)) return nullptr;
  return &kCurrentGrid->tilemap[pos.y][pos.x];
}

// Returns kTileBlock for non-existent tiles, and TileType otherwise.
bool
TileBlockedSafe(const v2i& pos)
{
  Tile* tile = TilePtr(pos);
  return !tile || tile->blocked;
}

// Returns any neighbor of type kTileOpen
v2i
TileOpenAdjacent(const v2i pos)
{
  Tile* tile = TilePtr(pos);
  assert(tile);
  if (!tile->blocked) return pos;

  for (int i = 0; i < kMaxNeighbor; ++i) {
    v2i neighbor = pos + kNeighbor[i];
    tile = TilePtr(neighbor);
    if (!tile) continue;

    if (!tile->blocked) return neighbor;
  }

  return INVALID_TILE;
}

v3f
TileAvoidWalls(const v2i pos)
{
  v2i avoidance = {};
  for (int i = 0; i < kMaxNeighbor; ++i) {
    v2i neighbor = pos + kNeighbor[i];
    Tile* tile = TilePtr(neighbor);
    if (!tile) continue;
    if (tile->blocked) {
      v2i away = (pos - neighbor);
      avoidance += away;
    }
  }

  return v3f(avoidance.x, avoidance.y, 0.0f);
}

v2f
TileVacuum(const v2i pos)
{
  v2f posf(pos.x * 1.f, pos.y * 1.f);
  v2f center(kMapWidth * 0.5f, kMapHeight * 0.5f);
  return Normalize(posf - center);
}

v2f
TileRandomPosition()
{
  v2i random_tile(rand() % kMapWidth, rand() % kMapHeight);
  return TilePosToWorld(random_tile);
}

v2i
TileRandomNeighbor(v2i tile)
{
  v2i n;
  do {
    int idx = rand() % kMaxNeighbor;
    n = tile + kNeighbor[idx];
  } while (!TileOk(n));
  return n;
}

math::Rectf
TilemapWorldBounds()
{
  math::Rectf ret;
  v2f minf = TilePosToWorldMin({0, 0});
  v2f maxf = TilePosToWorldMin({kMapWidth, kMapHeight});

  ret.x = minf.x;
  ret.y = minf.y;
  ret.width = maxf.x - minf.x;
  ret.height = maxf.y - minf.y;

  return ret;
}

v3f
TilemapWorldCenter()
{
  return TilePosToWorld({0, 0}) +
         v2f(kMapWidth * kTileWidth * .5f, kMapHeight * kTileHeight * .5f);
}

void
TilemapSet(uint64_t grid_idx)
{
  if (grid_idx == kInvalidIndex) return;

  if (grid_idx > kUsedGrid) {
    kCurrentGrid = nullptr;
    return;
  }

  kCurrentGrid = &kGrid[grid_idx];
  kTilemapWorldOffset = kCurrentGrid->transform.position.xy();
}

class TilemapModify {
 public:
  TilemapModify(uint64_t set) {
    prev_grid = kCurrentGrid;
    prev_offset = kTilemapWorldOffset;
    TilemapSet(set);
  }

  ~TilemapModify() {
    kCurrentGrid = prev_grid;
    kTilemapWorldOffset = prev_offset;
  }

  Grid* prev_grid;
  v2f prev_offset;
};

uint64_t
TilemapWorldToGrid(v3f world)
{
  for (int i = 0; i < kUsedGrid; ++i) {
    auto* grid = &kGrid[i];
    math::Rectf r(grid->transform.position.x, grid->transform.position.y,
                  kMapWidth * kTileWidth, kMapHeight * kTileHeight);
    if (math::PointInRect(world.xy(), r)) {
      return i;
    }
  }
  return kInvalidIndex;
}

bool
WorldToTilePos(const v3f pos, v2i* t)
{
  uint64_t tidx = TilemapWorldToGrid(pos);
  if (tidx == kInvalidIndex) return false;
  v2f offset = kGrid[tidx].transform.position.xy();
  v2f relpos = pos.xy() - offset;
  *t = v2i((int)(relpos.x * kInverseTileWidth),
           (int)(relpos.y * kInverseTileHeight));
  return true;
}

uint64_t
TilemapInitialize(TilemapType type)
{
  Grid* grid = UseGrid();
  uint64_t grid_index = grid - kGrid;
  grid->transform.position = v3f();
  TilemapSet(grid_index);

  // clang-format off
static int kDefaultMap[kMapHeight][kMapWidth] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 1, 1, 0, 0, 0, 0, 0, 0},
  {0, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

  // clang-format on
  switch (type) {
    case kTilemapEmpty: {
      kCurrentGrid->fog = false;
      for (int y = 0; y < kMapHeight; ++y) {
        for (int x = 0; x < kMapWidth; ++x) {
          Tile* tile = TilePtr(v2i(x, y));
          *tile = {};
          tile->cx = x;
          tile->cy = y;
          tile->explored = 1;
        }
      }
    } break;
    case kTilemapShip: {
      kCurrentGrid->fog = false;
      for (int y = kMapHeight-1; y >= 0; --y) {
        for (int x = 0; x < kMapWidth; ++x) {
          Tile* tile = TilePtr(v2i(x, y));
          *tile = {};
          tile->cx = x;
          tile->cy = y;
          tile->blocked = (kDefaultMap[y][x] == kTileBlock);
          tile->nooxygen = 0;
          tile->explored = 1;
          tile->exterior = 1;
          tile->shroud = 0;

          // Consumables enabled: cryo chamber, gatherable resources
          if (kDefaultMap[y][x] == kTileConsumable) {
            Consumable* c = UseConsumable();
            c->ship_index = grid_index;
            c->cx = tile->cx;
            c->cy = tile->cy;
            c->minerals = y * x % 89;
            if (kDefaultMap[y][x - 1] == kTileBlock) {
              c->cryo_chamber = 1;
            }
          }

          switch (kDefaultMap[y][x]) {
            case kTilePower:
            case kTileEngine:
            case kTileMine:
            case kTileTurret: {
              Module* t = UseModule();
              t->ship_index = grid_index;
              t->tile = v2i(x, y);
              t->mkind = (ModuleKind)(kDefaultMap[y][x] - kTileModule);
            } break;
          };
        }
      }
    } break;
  }

  return grid_index;
}  // namespace simulation

void
TilemapUpdate()
{
  if (!kCurrentGrid->fog) return;
  for (int i = 0; i < kMapHeight; ++i) {
    for (int j = 0; j < kMapWidth; ++j) {
      Tile* tile = TilePtr(v2i(j, i));
      tile->shroud = !tile->exterior;
    }
  }
}

}  // namespace simulation
