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
};

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
        queue.push({neighbor, t.second + 1});
        seen.insert(neighbor->grid_pos);
        results.push_back(neighbor);
      }
    }
    return results;
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
