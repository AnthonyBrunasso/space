#pragma once

// Don't include this file directly - include search.cc instead.
// This is contained in the search namespace

enum BfsIteratorFlag {
  // Don't allow searching that would travel diagnol through a blocked tile.
  // For example if travling from 5,2 to 6,3 but 5,3 is blocked.
  kAvoidBlockedDiagnol,
};

typedef bool IsBlockedCallback(const v2i&);
typedef v2i NeighborCallback(const v2i&, uint32_t);

struct BfsIterator {
  IsBlockedCallback* blocked_callback = nullptr;
  NeighborCallback* neighbor_callback = nullptr;
  uint32_t max_neighbor;
  v2i current;
  v2i map_size;
  // Depth of starting node is 0. Each expansion of bfs adds 1 to the depth
  // of the node it recursed from.
  uint32_t depth;
  uint32_t max_depth = UINT32_MAX;
  uint32_t flags;

  // Mostly meant for internal usage.
  int neighbor_index = 0;
  int queue_index = 0;
};

INLINE bool
IsValidPos(BfsIterator* itr)
{
  bool in_map = itr->current.x < itr->map_size.x && 
                itr->current.y < itr->map_size.y &&
                itr->current.x >= 0 && itr->current.y >= 0;
  if (!in_map) return false;
  if (itr->max_depth != UINT32_MAX && itr->depth > itr->max_depth) {
    return false;
  }
  return true;
}

INLINE bool
BfsStart(BfsIterator* itr)
{
  assert(itr);
  assert(itr->blocked_callback);
  assert(itr->neighbor_callback);
  assert(itr->map_size.x > 0 && itr->map_size.y > 0);
  assert(itr->max_neighbor);
  if (!IsValidPos(itr)) return false;
  kSearch = {};
  kSearch.queue_size = 0;
  itr->neighbor_index = 0;
  itr->queue_index = 0;
  kSearch.queue[kSearch.queue_size++] = itr->current;
  kSearch.path_map[itr->current.x][itr->current.y].checked = true;
  return true;
}

INLINE bool
BfsStep(const v2i& from, BfsIterator* itr)
{
  const auto& path_map = kSearch.path_map;
  const int neighbor_index = itr->neighbor_index;
  v2i neighbor = itr->neighbor_callback(from, neighbor_index);

  itr->queue_index = (neighbor_index + 1) / itr->max_neighbor;
  itr->neighbor_index += 1;
  itr->current = neighbor;
  itr->depth = path_map[from.x][from.y].depth + 1;

  return IsValidPos(itr);
}

INLINE bool
BfsNext(BfsIterator* itr)
{
  auto& queue = kSearch.queue;
  auto& path_map = kSearch.path_map;
  int& qsz = kSearch.queue_size;
  while (itr->queue_index < qsz) {
    v2i from = queue[itr->queue_index];
    if (BfsStep(from, itr)) {
      path_map[itr->current.x][itr->current.y].from = from;
      path_map[itr->current.x][itr->current.y].depth =
          path_map[from.x][from.y].depth + 1;
      queue[qsz++] = itr->current;
      break;
    }
  }
  return itr->queue_index < qsz;
}
