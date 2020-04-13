
#include "math/math.cc"

#include "tilemap.cc"

namespace simulation
{
Rectf
ShipBounds(uint64_t ship_index)
{
  Rectf bounds = {};
  if (ship_index >= kUsedShip) return bounds;

  Ship* ship = &kShip[ship_index];
  bounds.x = ship->transform.position.x;
  bounds.y = ship->transform.position.y;
  bounds.width = kMapWidth * kTileWidth;
  bounds.height = kMapHeight * kTileHeight;

  return bounds;
}

Rectf
FromShip(Tile tile)
{
  Rectf tile_bounds = {};
  if (!TileValid(tile)) return tile_bounds;
  const uint64_t ship_index = tile.ship_index;
  assert(ship_index < kUsedShip);

  v3f ship_position = kShip[ship_index].transform.position;
  tile_bounds.x = ship_position.x + (tile.cx*kTileWidth);
  tile_bounds.y = ship_position.y + (tile.cy*kTileHeight);
  tile_bounds.width = kTileWidth;
  tile_bounds.height = kTileHeight;

  return tile_bounds;
}

Tile
ToShip(uint64_t ship_index, v3f world)
{
  Tile tile = kZeroTile;

  if (!math::PointInRect(world.xy(), ShipBounds(ship_index))) return tile;

  TilemapModify tm(ship_index);

  v2f relpos = world.xy() - kShip[ship_index].transform.position.xy();
  v2i grid(relpos.x / kTileWidth, relpos.y / kTileHeight);
  tile = kCurrentGrid->tilemap[grid.y][grid.x];
  return tile; 
}

Tile
ToAnyShip(v3f world)
{
  for (int i = 0; i < kUsedShip; ++i) {
    Tile tile = ToShip(i, world);
    if (TileValid(tile)) return tile;
  }
  
  return kZeroTile;
}

Rectf
FleetBounds()
{
  // Find min and max bounds of all grids.
  v2f min(FLT_MAX, FLT_MAX);
  v2f max_dims(0.0f, 0.0f);
  for (int i = 0; i < kUsedShip; ++i) {
    Rectf bounds = ShipBounds(i);

    min.x = fmin(bounds.x, min.x);
    min.y = fmin(bounds.y, min.y);

    max_dims.x = fmax(bounds.width, max_dims.x);
    max_dims.y = fmax(bounds.height, max_dims.y);
  }

  return Rectf(min.x, min.y, max_dims.x, max_dims.y);
}

}  // namespace simulation
