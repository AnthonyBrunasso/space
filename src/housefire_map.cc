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

enum TileFlags {
  kTileDestination,
};

struct Tile {
  Tile() = default;
  Tile(uint32_t turns_to_fire)
      : turns_to_fire(turns_to_fire), turns_to_fire_max(turns_to_fire) {}
  v2i position_map;
  v3f position_world;
  v3f dims;
  uint32_t turns_to_fire;
  uint32_t turns_to_fire_max;
  uint32_t flags;
};

constexpr uint32_t kDefaultMapX = 5;
constexpr uint32_t kDefaultMapY = 5;

static Tile kDefaultMap[kDefaultMapX][kDefaultMapY] =
  {{5, 5, 5, 5, 3},
   {5, 5, 2, 6, 4},
   {5, 5, 1, 7, 5},
   {5, 5, 1, 8, 6},
   {5, 5, 1, 9, 5}};

static Tile kMap[kMapMaxX][kMapMaxY];

static char kEditMapName[64];
static uint32_t kMapNum = 0;

v3f
TilePosToWorld(const v2i& position_map)
{
  return {position_map.x * kTileWidth + 1.f * position_map.x,
          position_map.y * kTileHeight + 1.f * position_map.y, -kTileDepth};
}

void
MapUniqueNameFinder(const char* filename)
{
  if (strncmp(filename, "asset/level", 11) == 0) {
    uint32_t lvl_num;
    sscanf(filename, "asset/level_%u.map", &lvl_num);
    if (lvl_num >= kMapNum) {
      ++kMapNum;
    }
  }
  strcpy(kEditMapName, "asset/level_");
  char num[8];
  sprintf(num, "%d", kMapNum);
  strcat(kEditMapName, num);
  strcat(kEditMapName, ".map");
}

void
MapGenerateUniqueName()
{
  filesystem::WalkDirectory("asset/", MapUniqueNameFinder);
}

void
MapInitialize(uint32_t starting_ttf)
{
  for (int i = 0; i < kMapX; ++i) {
    for (int j = 0; j < kMapY; ++j) {
      Tile* tile = &kMap[i][j];
      tile->turns_to_fire_max = starting_ttf;
      tile->turns_to_fire = starting_ttf;
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
  memset(&kMap, 0, sizeof(Tile) * kMapMaxX * kMapMaxY);
  FILE* f = fopen(fname, "rb");
  if (!f) {
    // Load default map
    for (int i = 0; i < kDefaultMapX; ++i) {
      for (int j = 0; j < kDefaultMapY; ++j) {
        Tile* tile = &kMap[i][j];
        tile->turns_to_fire_max = kDefaultMap[i][j].turns_to_fire;
        tile->turns_to_fire = kDefaultMap[i][j].turns_to_fire;
        tile->position_map = v2i(i, j);
        tile->position_world = TilePosToWorld(tile->position_map);
        tile->dims = v3f(kTileWidth, kTileHeight, kTileDepth);
        kMapWidth = MAXF(kMapWidth, tile->position_world.x);
        kMapHeight = MAXF(kMapHeight, tile->position_world.y);
      }
    }
    kMapX = kDefaultMapX;
    kMapY = kDefaultMapY;
    return;
  }
  char line[32];
  char map_name[64] = {};
  char header[4] = {};
  fscanf(f, "%s %s %u %u\n", &header, &map_name, &kMapX, &kMapY);
  assert(strcmp(header, "map") == 0);
  v2i ctp;
  Tile* t = nullptr;
  while (1) {
    int res = fscanf(f, "%s", line);
    if (res == EOF) break;
    if (strcmp(line, "tile") == 0) {
      fscanf(f, "%u %u\n", &ctp.x, &ctp.y);
      t = &kMap[ctp.x][ctp.y];
    } else if (strcmp(line, "ttf") == 0) {
      assert(t);
      fscanf(f, "%u\n", &t->turns_to_fire_max);
      t->turns_to_fire = t->turns_to_fire_max;
      t->position_map = ctp;
      t->position_world = TilePosToWorld(t->position_map);
      t->dims = v3f(kTileWidth, kTileHeight, kTileDepth);
      kMapWidth = MAXF(kMapWidth, t->position_world.x);
      kMapHeight = MAXF(kMapHeight, t->position_world.y);
    } else if (strcmp(line, "flag") == 0) {
      assert(t);
      char flag_name[32] = {};
      fscanf(f, "%s\n", flag_name);
      if (strcmp(flag_name, "destination") == 0) {
        SBIT(t->flags, kTileDestination);
      }
    }
    else { continue; }  // Unrecognized line
  }
  fclose(f);
}

void
MapExport(const char* fname)
{
  printf("Exporting %s\n", fname);
  FILE* f = fopen(fname, "w+");
  if (!f) return;
  fprintf(f, "map %s %i %i\n", fname, kMapX, kMapY);
  for (int i = 0; i < kMapX; ++i) {
    for (int j = 0; j < kMapY; ++j) {
      Tile* tile = &kMap[i][j];
      fprintf(f, "tile %i %i\n", tile->position_map.x, tile->position_map.y);
      fprintf(f, "ttf %i\n", tile->turns_to_fire_max);
      if (FLAGGED(tile->flags, kTileDestination)) {
        fprintf(f, "flag destination\n");
      }
    }
  }
  fclose(f);
}

void
MapSetNextLevel(char* current_map)
{
  uint32_t lvl_num;
  sscanf(current_map, "asset/level_%u.map", &lvl_num);
  lvl_num++;
  char num[8];
  sprintf(num, "%d", lvl_num);
  strcpy(current_map, "asset/level_");
  strcat(current_map, num);
  strcat(current_map, ".map");
}


