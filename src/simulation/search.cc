#pragma once

#include <cstring>

#include "tilemap.cc"

namespace search
{
struct PathNode {
  math::Vec2i from;
  bool checked;
};

struct Path {
  math::Vec2i tile[tilemap::kMapHeight * tilemap::kMapWidth];
  int size;
};

struct Search {
  // Map used for pathfinding calculations.
  PathNode path_map[tilemap::kMapHeight][tilemap::kMapWidth];
  // BFS queue.
  math::Vec2i queue[tilemap::kMapHeight * tilemap::kMapWidth];
  int queue_size;
  int queue_ptr;
  // The resulting path as calculated from the last call to PathTo.
  Path path;
};

static Search kSearch;

Path*
PathTo(const math::Vec2i& start, const math::Vec2i& end)
{
  if (!tilemap::TileOk(end)) return nullptr;
  if (!tilemap::TileOk(start)) return nullptr;

  constexpr int N = tilemap::kMapHeight * tilemap::kMapWidth;
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

    for (int i = 0; i < tilemap::kMaxNeighbor; ++i) {
      const math::Vec2i neighbor = node + tilemap::kNeighbor[i];
      if (tilemap::TileTypeSafe(neighbor) != tilemap::kTileOpen) continue;

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
BfsReplace(math::Vec2i start, const uint64_t limit, tilemap::TileType seek,
           tilemap::TileType set)
{
  uint64_t count = 0;
  if (!tilemap::TileOk(start)) return count;
  if (tilemap::kTilemap.map[start.y][start.x].type == seek) {
    tilemap::kTilemap.map[start.y][start.x].type = set;
    ++count;
  }

  constexpr int N = tilemap::kMapHeight * tilemap::kMapWidth;
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
    for (int i = 0; i < tilemap::kMaxNeighbor; ++i) {
      const math::Vec2i neighbor = node + tilemap::kNeighbor[i];
      if (!tilemap::TileOk(neighbor)) continue;
      if (path_map[neighbor.y][neighbor.x].checked == true) continue;
      path_map[neighbor.y][neighbor.x].checked = true;
      path_map[neighbor.y][neighbor.x].from = node;

      if (tilemap::kTilemap.map[neighbor.y][neighbor.x].type == seek) {
        tilemap::kTilemap.map[neighbor.y][neighbor.x].type = set;
        ++count;
      }

      if (tilemap::kTilemap.map[neighbor.y][neighbor.x].type !=
          tilemap::kTileBlock) {
        queue[qsz++] = neighbor;
      }
    }
  }

  return count;
}

}  // namespace search
