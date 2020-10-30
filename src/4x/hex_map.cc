#pragma once

#include <unordered_map>
#include <unordered_set>
#include <queue>

// Blergh
namespace std {

template <>
struct hash<v2i>
{
  std::size_t
  operator()(const v2i& grid) const
  {
    // Arbitrarily large prime numbers
    const size_t h1 = 0x8da6b343;
    const size_t h2 = 0xd8163841;
    return grid.x * h1 + grid.y * h2;
  }
};

}  // namespace std

namespace fourx {

struct HexTile {
  HexTile(const v2i& grid_pos) : grid_pos(grid_pos) {}
  v2i grid_pos;
  b8 blocked = false;
};


template <typename T>
void
ClearQueue(std::queue<T>& q)
{
  std::queue<T> empty;
  std::swap(q, empty);
}

class HexMap {
 public:
  explicit
  HexMap(u32 sz)
  {
    std::vector<v2i> grids = math::HexAxialRange(sz);
    tiles_.reserve(grids.size());
    for (v2i g : grids) {
      tiles_.push_back(HexTile(g));
      tile_map_[g] = &tiles_.back();
    }
  }

  std::vector<HexTile*>
  Bfs(const v2i& start, u32 depth)
  {
    HexTile* start_tile = tile(start);
    if (!start_tile) return {};
    std::queue<std::pair<HexTile*, u32>> queue;
    std::unordered_set<v2i> seen;
    queue.push({start_tile, 0});
    std::vector<HexTile*> results;
    results.push_back(start_tile);
    while (!queue.empty()) {
      std::pair<HexTile*, u32> t = queue.front();
      queue.pop();
      if (t.second >= depth) continue;
      seen.insert(t.first->grid_pos);
      for (s32 i = 0; i < 6; ++i) {
        v2i grid = math::HexAxialNeighbor(t.first->grid_pos, i);
        HexTile* neighbor = tile(grid);
        if (!neighbor) continue;
        if (seen.find(neighbor->grid_pos) != seen.end()) continue;
        seen.insert(neighbor->grid_pos);
        if (neighbor->blocked) continue;
        queue.push({neighbor, t.second + 1});
        results.push_back(neighbor);
      }
    }
    return results;
  }

  std::vector<HexTile*>
  BfsPathTo(const v2i& start, const v2i& end)
  {
    HexTile* start_tile = tile(start);
    if (!start_tile) return {};
    std::queue<HexTile*> queue;
    std::unordered_map<v2i, v2i> path_map;
    std::unordered_set<v2i> seen;
    queue.push(start_tile);
    while (!queue.empty()) {
      HexTile* t = queue.front();
      queue.pop();
      seen.insert(t->grid_pos);
      for (s32 i = 0; i < 6; ++i) {
        v2i grid = math::HexAxialNeighbor(t->grid_pos, i);
        HexTile* neighbor = tile(grid);
        if (!neighbor) continue;
        if (seen.find(neighbor->grid_pos) != seen.end()) continue;
        seen.insert(neighbor->grid_pos);
        if (neighbor->blocked) continue;
        queue.push(neighbor);
        path_map[neighbor->grid_pos] = t->grid_pos;
        if (neighbor->grid_pos == end) {
          ClearQueue(queue);
          break;
        }
      }
    }

    // Missing end node means a path was never found.
    auto enditr = path_map.find(end);
    if (enditr == path_map.end()) return {};

    std::vector<HexTile*> path;
    path.push_back(tile(enditr->first));
    while (path.back()->grid_pos != start) {
      path.push_back(tile(path_map[path.back()->grid_pos]));
    }
    std::reverse(path.begin(), path.end());
    return path;
  }

  HexTile*
  tile(const v2i& grid)
  {
    auto found = tile_map_.find(grid);
    if (found == tile_map_.end()) return nullptr;
    return found->second;
  }

  const std::vector<HexTile>&
  tiles() const
  {
    return tiles_;
  }

 private:
  // All of the tiles in a dense vector.
  std::vector<HexTile> tiles_;
  // Pointers to tile in map so tiles can be looked up quickly.
  std::unordered_map<v2i, HexTile*> tile_map_;
};

}  // namespace 4x
