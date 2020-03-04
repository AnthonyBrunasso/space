#pragma once

#include <cstring>

#include "tilemap.cc"

namespace simulation
{
struct PathNode {
  v2i from;
  bool checked;
};

struct Path {
  v2i tile[kMapHeight * kMapWidth];
  int size;
};

struct Search {
  // Map used for pathfinding calculations.
  PathNode path_map[kMapHeight][kMapWidth];
  // BFS queue.
  v2i queue[kMapHeight * kMapWidth];
  int queue_size;
  int queue_ptr;
  // The resulting path as calculated from the last call to PathTo.
  Path path;
};

static Search kSearch;

void
BfsStart(v2i start)
{
  constexpr int N = kMapHeight * kMapWidth;
  memset(kSearch.path_map, 0, sizeof(PathNode) * N);
  kSearch.queue_size = 0;
  kSearch.queue_ptr = 0;
  kSearch.path.size = 0;

  auto& queue = kSearch.queue;
  int& qsz = kSearch.queue_size;
  int& qptr = kSearch.queue_ptr;
  auto& path_map = kSearch.path_map;

  queue[qsz++] = start;
  path_map[start.y][start.x].from = start;
  path_map[start.y][start.x].checked = true;
}

bool
BfsStep(int i, v2i* n)
{
  auto& queue = kSearch.queue;
  int& qsz = kSearch.queue_size;
  int& qptr = kSearch.queue_ptr;
  auto& path_map = kSearch.path_map;
  auto& from = queue[qptr];
  if (i >= kMaxNeighbor - 1) ++qptr;
  const v2i neighbor = from + kNeighbor[i];
  Tile* neighbor_tile = TilePtr(neighbor);
  if (!neighbor_tile || neighbor_tile->blocked) return false;
  if (path_map[neighbor.y][neighbor.x].checked == true) return false;
  path_map[neighbor.y][neighbor.x].checked = true;
  path_map[neighbor.y][neighbor.x].from = from;
  *n = neighbor;
  return true;
}

Path*
PathTo(const v2i& start, const v2i& end)
{
  if (!TileOk(end)) return nullptr;
  if (!TileOk(start)) return nullptr;
  
  BfsStart(start);

  auto& queue = kSearch.queue;
  int& qsz = kSearch.queue_size;
  int& qptr = kSearch.queue_ptr;
  auto& path_map = kSearch.path_map;
  while (1) {
    // No Path
    if (qptr == qsz) return nullptr;
    // Path success
    if (queue[qptr] == end) break;
    v2i n;
    for (int i = 0; i < kMaxNeighbor; ++i) {
      if (BfsStep(i, &n)) queue[qsz++] = n;
    }
  }

  auto& path = kSearch.path;
  auto& psz = kSearch.path.size;
  path.tile[psz++] = end;
  while (path.tile[psz - 1] != start) {
    auto& prev = path.tile[psz - 1];
    path.tile[psz++] = path_map[prev.y][prev.x].from;
  }
  // Reverse it
  for (int i = 0, e = psz - 1; i < e; ++i, --e) {
    auto t = path.tile[e];
    path.tile[e] = path.tile[i];
    path.tile[i] = t;
  }
#if 0
  printf("Path is\n\n");
  for (int i = 0; i < psz; ++i) {
    printf("(%i,%i)\n", path.tile[i].x, path.tile[i].y);
  }
#endif
  return &kSearch.path;
}

uint64_t
BfsRemoveOxygen(v2i start, const uint64_t limit)
{
  uint64_t count = 0;
  {
    Tile* start_tile = TilePtr(start);
    if (!start_tile) return count;
    if (start_tile->nooxygen == 0) {
      start_tile->nooxygen = 1;
      ++count;
    }
  }

  BfsStart(start);

  auto& queue = kSearch.queue;
  int& qsz = kSearch.queue_size;
  int& qptr = kSearch.queue_ptr;
  auto& path_map = kSearch.path_map;

  while (count < limit) {
    // No more tiles
    if (qptr == qsz) return count;
    v2i n;
    for (int i = 0; i < kMaxNeighbor; ++i) {
      if (BfsStep(i, &n)) {
        Tile* neighbor_tile = TilePtr(n);
        if (neighbor_tile->nooxygen == 0) {
          neighbor_tile->nooxygen = 1;
          ++count;
        }
        queue[qsz++] = n;
      }
    }
  }

  return count;
}

void
BfsMutate(v3f origin, Tile keep_bits, Tile set_bits, float tile_dsq)
{
  v2i start = WorldToTilePos(origin);
  {
    Tile* start_tile = TilePtr(start);
    if (!start_tile) return;
    *start_tile = TileAND(*start_tile, keep_bits);
    *start_tile = TileOR(*start_tile, set_bits);
  }

  BfsStart(start);

  auto& queue = kSearch.queue;
  int& qsz = kSearch.queue_size;
  int& qptr = kSearch.queue_ptr;
  auto& path_map = kSearch.path_map;

  while (1) {
    // No more tiles
    if (qptr == qsz) return;
    v2i n;
    for (int i = 0; i < kMaxNeighbor; ++i) {
      if (BfsStep(i, &n)) {
        Tile* neighbor_tile = TilePtr(n);
        v3f neighbor_world = TilePosToWorld(n);
        v3f world = neighbor_world - origin;
        float distance = LengthSquared(world);
        if (distance > tile_dsq) continue;

        if (!neighbor_tile->blocked) {
          *neighbor_tile = TileAND(*neighbor_tile, keep_bits);
          *neighbor_tile = TileOR(*neighbor_tile, set_bits);
          queue[qsz++] = n;
        }
      }
    }
  }
}

}  // namespace simulation
