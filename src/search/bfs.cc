#pragma once

// Don't include this file directly - include search.cc instead.
// This is contained in the search namespace

enum BfsIteratorFlag {
  // Don't allow searching that would travel diagnol through a blocked tile.
  // For example if travling from 5,2 to 6,3 but 5,3 is blocked.
  kAvoidBlockedDiagnol,
};

typedef b8 IsBlockedCallback(const v2i&);
typedef v2i NeighborCallback(const v2i&, u32);
typedef PathmapNode* GetPathmapNode(const v2i&);

PathmapNode*
GetPathmapNodeDefault(const v2i& v)
{
  return &kSearch.path_map[v.x][v.y];
}

struct BfsIterator {
  IsBlockedCallback* blocked_callback = nullptr;
  NeighborCallback* neighbor_callback = nullptr;
  GetPathmapNode* get_pathmap_node = GetPathmapNodeDefault;
  u32 max_neighbor;
  v2i current;
  v2i map_size;
  // Depth of starting node is 0. Each expansion of bfs adds 1 to the depth
  // of the node it recursed from.
  u32 depth = 0;
  u32 max_depth = UINT32_MAX;
  u32 flags;

  // Mostly meant for internal usage.
  s32 neighbor_index = 0;
  s32 queue_index = 0;
};

INLINE b8
IsInMap(const v2i& pos, const v2i& map_size)
{
  return true;
}

INLINE b8
IsValidPos(const BfsIterator* itr)
{
  if (!IsInMap(itr->current, itr->map_size)) return false;
  if (itr->max_depth != UINT32_MAX && itr->depth > itr->max_depth) {
    return false;
  }
  return true;
}

INLINE b8
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
  return true;
}

INLINE b8
BfsStep(const v2i& from, BfsIterator* itr)
{
  const s32 neighbor_index = itr->neighbor_index;
  printf("%s from [%i %i] idx %i\n", __func__, from.x, from.y, neighbor_index);
  v2i neighbor = itr->neighbor_callback(from, neighbor_index);

  itr->queue_index = (neighbor_index + 1) / itr->max_neighbor;
  itr->neighbor_index += 1;
  if (itr->neighbor_index >= itr->max_neighbor) itr->neighbor_index = 0;
  itr->current = neighbor;
  auto* pnode = itr->get_pathmap_node(from);
  printf("%s From [%i %i] is [%s]\n",
         __func__, from.x, from.y,
         pnode == nullptr ? "NULL" : "SET");
  if (!pnode) return false;
  itr->depth = pnode->depth + 1;

  return IsValidPos(itr) && !pnode->checked;
}

INLINE b8
BfsNext(BfsIterator* itr)
{
  printf("%s 1\n", __func__);
  auto& queue = kSearch.queue;
  auto& path_map = kSearch.path_map;
  s32& qsz = kSearch.queue_size;
  while (itr->queue_index < qsz) {
    printf("%s 2\n", __func__);
    v2i from = queue[itr->queue_index];
    if (BfsStep(from, itr)) {
      if (itr->blocked_callback(itr->current)) continue;
      if (FLAGGED(itr->flags, kAvoidBlockedDiagnol)) {
        b8 right_tile_blocked = IsInMap(from + v2i(1, 0), itr->map_size) &&
                                itr->blocked_callback(from + v2i(1, 0));
        b8 left_tile_blocked = IsInMap(from + v2i(-1, 0), itr->map_size) &&
                               itr->blocked_callback(from + v2i(-1, 0));
        b8 bottom_tile_blocked = IsInMap(from + v2i(0, -1), itr->map_size) &&
                                 itr->blocked_callback(from + v2i(0, -1));
        b8 top_tile_blocked = IsInMap(from + v2i(0, 1), itr->map_size) &&
                                      itr->blocked_callback(from + v2i(0, 1));
        if (itr->current.x == from.x + 1 && itr->current.y == from.y + 1 &&
            (right_tile_blocked || top_tile_blocked)) {
          continue;
        }
        if (itr->current.x == from.x + 1 && itr->current.y == from.y - 1 &&
            (right_tile_blocked || bottom_tile_blocked)) {
          continue;
        }
        if (itr->current.x == from.x - 1 && itr->current.y == from.y + 1 &&
            (left_tile_blocked || top_tile_blocked)) {
          continue;
        }
        if (itr->current.x == from.x - 1 && itr->current.y == from.y - 1 &&
            (left_tile_blocked || bottom_tile_blocked)) {
          continue;
        }
      }
      PathmapNode* node = itr->get_pathmap_node(itr->current);
      printf("Node is %i %i is [%s]\n", itr->current.x, itr->current.y,
             node == nullptr ? "NULL" : "SET");
      if (!node) return false;
      node->from = from;
      node->depth = itr->get_pathmap_node(from)->depth + 1;
      node->checked = true;
      queue[qsz++] = itr->current;
      break;
    }
  }
  return itr->queue_index < qsz;
}

INLINE Path*
BfsPathTo(BfsIterator* itr, const v2i& end)
{
  v2i start = itr->current;
  if (itr->current == end) return nullptr;

  auto& path_map = kSearch.path_map;
  BfsStart(itr);
  s32 i = 0;
  printf("%s\n", __func__);
  while (BfsNext(itr)) {
    // A depth value greater than 0 means the end node was expanded.
    if (itr->get_pathmap_node(end)->depth) {
      break;
    }
  }

  auto* node = itr->get_pathmap_node(end);
  if (!node || !node->depth) return nullptr;

  kPath = {};
  auto& path = kPath.queue;
  auto& psz = kPath.size;
  path[psz++] = end;
  while (path[psz - 1] != start) {
    auto& prev = path[psz - 1];
    path[psz++] = itr->get_pathmap_node(prev)->from;
  }
  // Reverse it
  for (s32 i = 0, last = psz - 1; i < last; ++i, --last) {
    auto t = path[last];
    path[last] = path[i];
    path[i] = t;
  }

  return &kPath;
}
