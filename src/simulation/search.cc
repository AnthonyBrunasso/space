#pragma once

#include <cstring>

#include "ship.cc"

namespace simulation
{
struct Path {
  Tile tile[kMapMaxHeight * kMapMaxWidth];
  int size;
};

struct Search {
  // Visited Nodes
  Tile path_map[kMapMaxHeight][kMapMaxWidth];
  // BFS queue.
  Tile queue[kMapMaxHeight * kMapMaxWidth];
  int queue_size;
  // The resulting path as calculated from the last call to PathTo.
  Path path;
};

struct BfsIterator {
  int neighbor_index = 0;
  int queue_index = 0;
  Tile* tile = nullptr;
};

static Search kSearch;

BfsIterator
BfsStart(Tile tile)
{
  memset(kSearch.path_map, 0, sizeof(kSearch.path_map));
  kSearch.queue_size = 0;
  kSearch.path.size = 0;
  BfsIterator itr;
  itr.neighbor_index = 0;
  itr.queue_index = 0;
  itr.tile = nullptr;
  if (!TileValid(tile)) return itr;
  kSearch.queue[kSearch.queue_size++] = tile;
  kSearch.path_map[tile.cy][tile.cx] = tile;
  itr.tile = ShipTile(tile);
  return itr;
}

// Performs a single step of the bfs expansion.
// Returns true when the next node is first discovered.
// Returns false when the next node does not exist, or has been seen.
INLINE bool
BfsStep(Tile from, BfsIterator* iter)
{
  const auto& path_map = kSearch.path_map;
  const int neighbor_index = iter->neighbor_index;
  Tile neighbor = TileNeighbor(from, neighbor_index);
  Tile* tile = ShipTile(neighbor);

  iter->queue_index = (neighbor_index + 1) / kMaxNeighbor;
  iter->neighbor_index += 1;
  iter->tile = tile;

  if (TileValid(path_map[neighbor.cy][neighbor.cx])) return false;
  return true;
}

// Find a tile that has not been seen, respecting the "blocked" tile flag.
//
// Returns true when more nodes may be searched.
// Returns false when all nodes have been searched.
INLINE bool
BfsNextTile(BfsIterator* iter)
{
  auto& queue = kSearch.queue;
  auto& path_map = kSearch.path_map;
  int& qsz = kSearch.queue_size;

  while (iter->queue_index < qsz) {
    Tile from = queue[iter->queue_index];
    if (BfsStep(from, iter)) {
      if (iter->tile->blocked) continue;

      path_map[iter->tile->cy][iter->tile->cx] = from;
      queue[qsz++] = *iter->tile;
      break;
    }
  }

  return iter->queue_index < qsz;
}

// Find any node that has not been seen.
//
// Returns true when more nodes may be searched.
// Returns false when all nodes have been searched.
INLINE bool
BfsNext(BfsIterator* iter)
{
  auto& queue = kSearch.queue;
  auto& path_map = kSearch.path_map;
  int& qsz = kSearch.queue_size;

  while (iter->queue_index < qsz) {
    Tile from = queue[iter->queue_index];
    if (BfsStep(from, iter)) {
      path_map[iter->tile->cy][iter->tile->cx] = from;
      queue[qsz++] = *iter->tile;
      break;
    }
  }

  return iter->queue_index < qsz;
}

Path*
PathTo(Tile start, Tile end)
{
  if (start == end) return nullptr;

  auto& path_map = kSearch.path_map;
  BfsIterator iter = BfsStart(start);
  while (BfsNextTile(&iter)) {
    if (TileValid(path_map[end.cy][end.cx])) {
      break;
    }
  }

  if (!TileValid(path_map[end.cy][end.cx])) return nullptr;

  auto& path = kSearch.path;
  auto& psz = kSearch.path.size;
  path.tile[psz++] = end;
  while (path.tile[psz - 1] != start) {
    auto& prev = path.tile[psz - 1];
    path.tile[psz++] = kSearch.path_map[prev.cy][prev.cx];
  }
  // Reverse it
  for (int i = 0, last = psz - 1; i < last; ++i, --last) {
    auto t = path.tile[last];
    path.tile[last] = path.tile[i];
    path.tile[i] = t;
  }

  return &kSearch.path;
}

// set_tile contains the start location (cx, cy)
// set_tile contains the flags to be enabled during Bfs
// tile_dsq is tile distance squared
void
BfsTileEnable(Tile set_tile, uint64_t tile_distance)
{
  auto& queue = kSearch.queue;
  auto& path_map = kSearch.path_map;
  int& qsz = kSearch.queue_size;

  BfsIterator iter = BfsStart(set_tile);
  if (!iter.tile->blocked) {
    TileSet(iter.tile, set_tile.flags);
  }

  while (iter.queue_index != qsz) {
    Tile from = queue[iter.queue_index];
    if (BfsStep(from, &iter)) {
      if (iter.tile->blocked) continue;

      if (TileDsq(*iter.tile, set_tile, tile_distance)) {
        TileSet(iter.tile, set_tile.flags);

        path_map[iter.tile->cy][iter.tile->cx] = from;
        queue[qsz++] = *iter.tile;
      }
    }
  }
}

}  // namespace simulation
