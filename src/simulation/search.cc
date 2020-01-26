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

Path*
PathTo(const v2i& start, const v2i& end)
{
  if (!TileOk(end)) return nullptr;
  if (!TileOk(start)) return nullptr;

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

  while (1) {
    // No Path
    if (qptr == qsz) return nullptr;

    auto& node = queue[qptr++];
    // Path success
    if (node == end) break;

    for (int i = 0; i < kMaxNeighbor; ++i) {
      const v2i neighbor = node + kNeighbor[i];
      if (TileTypeSafe(neighbor) != kTileOpen) continue;

      if (path_map[neighbor.y][neighbor.x].checked == true) continue;
      path_map[neighbor.y][neighbor.x].checked = true;
      path_map[neighbor.y][neighbor.x].from = node;

      queue[qsz++] = neighbor;
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
BfsReplace(v2i start, const uint64_t limit, TileType seek, TileType set)
{
  uint64_t count = 0;
  if (!TileOk(start)) return count;
  if (kTilemap.map[start.y][start.x].type == seek) {
    kTilemap.map[start.y][start.x].type = set;
    ++count;
  }

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

  while (count < limit) {
    // No more tiles
    if (qptr == qsz) return count;

    auto& node = queue[qptr++];
    for (int i = 0; i < kMaxNeighbor; ++i) {
      const v2i neighbor = node + kNeighbor[i];
      if (!TileOk(neighbor)) continue;
      if (path_map[neighbor.y][neighbor.x].checked == true) continue;
      path_map[neighbor.y][neighbor.x].checked = true;
      path_map[neighbor.y][neighbor.x].from = node;

      if (kTilemap.map[neighbor.y][neighbor.x].type == seek) {
        kTilemap.map[neighbor.y][neighbor.x].type = set;
        ++count;
      }

      if (kTilemap.map[neighbor.y][neighbor.x].type != kTileBlock) {
        queue[qsz++] = neighbor;
      }
    }
  }

  return count;
}

}  // namespace simulation
