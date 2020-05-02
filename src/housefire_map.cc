#pragma once


constexpr uint32_t kMapMaxX = 32;
constexpr uint32_t kMapMaxY = 32;

uint32_t kMapX = 4;
uint32_t kMapY = 4;

static float kMapWidth = 0.f;
static float kMapHeight = 0.f;

constexpr float kTileWidth = 25.f;
constexpr float kTileHeight = 25.f;
constexpr float kTileDepth = 4.f;
constexpr int kMaxTileNeighbor = 8;

struct Tile {
  Tile() = default;
  Tile(uint32_t turns_to_fire)
      : turns_to_fire(turns_to_fire), turns_to_fire_max(turns_to_fire) {}
  v2i position_map;
  v3f position_world;
  v3f dims;
  uint32_t turns_to_fire;
  uint32_t turns_to_fire_max;
};

static Tile kDefaultMap[kMapMaxX][kMapMaxY] =
  {{5, 5, 5, 5, 5},
   {5, 5, 5, 5, 5},
   {5, 5, 5, 5, 5},
   {5, 5, 5, 5, 5},
   {5, 5, 5, 5, 5}};

static Tile kMap[kMapMaxX][kMapMaxY] = 
  {{5, 5, 5, 5},
   {5, 5, 2, 6},
   {5, 5, 1, 7},
   {5, 5, 1, 8}};

v3f
TilePosToWorld(const v2i& position_map)
{
  return {position_map.x * kTileWidth + 1.f * position_map.x,
          position_map.y * kTileHeight + 1.f * position_map.y, -kTileDepth};
}

void
MapInitialize()
{
  for (int i = 0; i < kMapX; ++i) {
    for (int j = 0; j < kMapY; ++j) {
      Tile* tile = &kMap[i][j];
      tile->position_map = v2i(i, j);
      tile->position_world = TilePosToWorld(tile->position_map);
      tile->dims = v3f(kTileWidth, kTileHeight, kTileDepth);
      kMapWidth = MAXF(kMapWidth, tile->position_world.x);
      kMapHeight = MAXF(kMapHeight, tile->position_world.y);
    }
  }
}

void
MapLoad(const char* fname)
{
}

void
MapExport(const char* fname)
{
  FILE* f = fopen(fname, "w+");
  if (!f) return;
  fprintf(f, "map %s %i %i\n", fname, kMapX, kMapY);
  for (int i = 0; i < kMapX; ++i) {
    for (int j = 0; j < kMapY; ++j) {
      Tile* tile = &kMap[i][j];
      fprintf(f, "tile %i %i\n", tile->position_map.x, tile->position_map.y);
      fprintf(f, "t %i\n", tile->turns_to_fire_max);
    }
  }
  fclose(f);
}
