#pragma once

namespace fourx {

std::optional<v2i>
GetHexMapv2i(const v2i& v, u32 size)
{
  v2i r = v + v2i(size, size);
  if (r.x < 0 || r.y < 0) return std::nullopt;
  if (r.x > 2 * size) return std::nullopt;
  if (r.y > 2 * size) return std::nullopt;
  if (r.x + r.y > size * 3) return std::nullopt;
  if (r.x + r.y < size) return std::nullopt;
  return r;
}

struct HexTile {
  HexTile(const v2i& grid_pos) : grid_pos(grid_pos) {}
  v2i grid_pos;
};

class HexMap {
 public:
  explicit
  HexMap(u32 sz)
      : map_size_(sz)
  {
    // Definitely some wasted space here for a contiguous array.
    tile_map_.resize((sz * 2) * (sz * 2), nullptr);
    std::vector<v2i> grids = math::HexAxialRange(sz);
    tiles_.reserve(grids.size());
    for (v2i g : grids) {
      tiles_.push_back(HexTile(g));
      std::optional<u32> tidx = GetMapIndex(g);
      assert(tidx);
      tile_map_[*tidx] = &tiles_.back();
    }
  }

  // Turn the tiles grid location into the index of tiles_
  std::optional<u32>
  GetMapIndex(const v2i& grid) const
  {
    // Turn the grid location non-negative.
    v2i r = grid + v2i(map_size_, map_size_);
    // Return nullopt if the grid location falls outside the bounds of the map
    if (r.x < 0 || r.y < 0) return std::nullopt;
    if (r.x > 2 * map_size_) return std::nullopt;
    if (r.y > 2 * map_size_) return std::nullopt;
    if (r.x + r.y > map_size_ * 3) return std::nullopt;
    if (r.x + r.y < map_size_) return std::nullopt;
    return r.x * map_size_ + r.y;
  }

  HexTile*
  tile(const v2i& grid)
  {
    std::optional<u32> idx = GetMapIndex(grid);
    if (!idx) return nullptr;
    return tile_map_[*idx];
  }

  const std::vector<HexTile>&
  tiles() const
  {
    return tiles_;
  }

 private:
  // All of the tiles in a dense vector.
  std::vector<HexTile> tiles_;
  // Pointers to tile in sparse vector that can be indexed into by grid loc.
  std::vector<HexTile*> tile_map_;
  u32 map_size_;
};



}  // namespace 4x
