#pragma once

#include <cstdio>

#include "math/vec.h"

namespace tilemap
{
constexpr int kMapWidth = 32;
constexpr int kMapHeight = 32;

constexpr float kTileWidth = 25.0f;
constexpr float kTileHeight = 25.0f;
constexpr int MAX_POD = 3;

enum TileType {
  kTileOpen = 0,
  kTileBlock = 1,
  kTileEngine = 2,
  kTilePower = 3,
  kTileMine = 4,
  kTileVacuum = 5,
  kTileTurret = 6,
};

struct Tile {
  math::Vec2i pos;
  TileType type;
};

struct Tilemap {
  Tile map[kMapHeight][kMapWidth];
};

constexpr int kMaxNeighbor = 8;
static const math::Vec2i kNeighbor[kMaxNeighbor] = {
    math::Vec2i(-1, 0), math::Vec2i(1, 0),   math::Vec2i(0, 1),
    math::Vec2i(0, -1), math::Vec2i(1, 1),   math::Vec2i(-1, 1),
    math::Vec2i(1, -1), math::Vec2i(-1, -1),
};

static Tilemap kTilemap;
static math::Vec2i kInvalidTile = math::Vec2i{-1, -1};

// clang-format off
static int kDefaultMap[kMapHeight][kMapWidth] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 6, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
  {0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
  {0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
  {0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
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

void
Initialize()
{
  for (int i = 0; i < kMapHeight; ++i) {
    for (int j = 0; j < kMapWidth; ++j) {
      Tile* tile = &kTilemap.map[i][j];
      tile->type = (TileType)kDefaultMap[i][j];
      tile->pos.x = j;
      tile->pos.y = i;
    }
  }
}

// Returns the center position of the tile.
math::Vec2f
TileToWorld(const Tile& tile)
{
  return {(tile.pos.x * kTileWidth) + kTileWidth / 2.f,
          (tile.pos.y * kTileHeight) + kTileHeight / 2.f};
}

// Returns the center position of the tile.
math::Vec2f
TilePosToWorld(const math::Vec2i& pos)
{
  return {((float)pos.x * kTileWidth) + kTileWidth / 2.f,
          ((float)pos.y * kTileHeight) + kTileHeight / 2.f};
}

math::Vec2i
WorldToTilePos(const math::Vec2f& pos)
{
  int x = (int)pos.x / kTileWidth;
  int y = (int)pos.y / kTileHeight;
  return {x, y};
}

// Returns true for positions that exist as a tile in kTilemap
bool
TileOk(math::Vec2i pos)
{
  if (pos.x < 0) return false;
  if (pos.x >= kMapWidth) return false;
  if (pos.y < 0) return false;
  if (pos.y >= kMapHeight) return false;
  return true;
}

// Returns kTileBlock for non-existent tiles, and TileType otherwise.
TileType
TileTypeSafe(const math::Vec2i& pos)
{
  if (!TileOk(pos)) return kTileBlock;
  return kTilemap.map[pos.y][pos.x].type;
}

// Returns any neighbor of type kTileOpen
math::Vec2i
TileOpenAdjacent(const math::Vec2i pos)
{
  if (TileTypeSafe(pos) == kTileOpen) return pos;

  for (int i = 0; i < kMaxNeighbor; ++i) {
    math::Vec2i neighbor = pos + kNeighbor[i];
    if (TileTypeSafe(neighbor) == kTileOpen) return neighbor;
  }

  return kInvalidTile;
}

math::Vec3f
TileAvoidWalls(const math::Vec2i pos)
{
  math::Vec2i avoidance = {};
  for (int i = 0; i < tilemap::kMaxNeighbor; ++i) {
    math::Vec2i neighbor = pos + tilemap::kNeighbor[i];
    if (TileTypeSafe(neighbor) != kTileOpen) {
      math::Vec2i away = (pos - neighbor);
      avoidance += away;
    }
  }

  return math::Vec3f(avoidance.x, avoidance.y, 0.0f);
}

math::Vec3f
TileVacuum(const math::Vec2i pos)
{
  math::Vec2i attraction = {};
  for (int i = 0; i < tilemap::kMaxNeighbor; ++i) {
    math::Vec2i neighbor = pos + tilemap::kNeighbor[i];
    if (TileTypeSafe(neighbor) != kTileOpen) {
      math::Vec2i toward = (neighbor - pos);
      attraction += toward;
    }
  }

  return math::Vec3f(attraction.x, attraction.y, 0.0f);
}

bool
TileTypeWorldPosition(TileType type, math::Vec2f* world)
{
  for (int i = 0; i < kMapHeight; ++i) {
    for (int j = 0; j < kMapWidth; ++j) {
      const Tile* tile = &kTilemap.map[i][j];
      if (tile->type == type) {
        math::Vec2i near_engine = TileOpenAdjacent(tile->pos);
        *world = TilePosToWorld(near_engine);
        return true;
      }
    }
  }

  return false;
}

}  // namespace tilemap
