#pragma once

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
  bounds.width = ship->map_width * kTileWidth;
  bounds.height = ship->map_height * kTileHeight;

  return bounds;
}

Rectf
FleetBounds()
{
  // Find min and max bounds of all grids.
  v2f min(FLT_MAX, FLT_MAX);
  v2f max(0.0f, 0.0f);
  for (int i = 0; i < kUsedShip; ++i) {
    Rectf bounds = ShipBounds(i);

    min.x = fmin(bounds.x, min.x);
    min.y = fmin(bounds.y, min.y);

    max.x = fmax(bounds.x + bounds.width, max.x);
    max.y = fmax(bounds.y + bounds.height, max.y);
  }

  return Rectf(min.x, min.y, max.x - min.x, max.y - min.y);
}

Rectf
FromShip(Tile tile)
{
  Rectf tile_bounds = {};
  if (!TileValid(tile)) return tile_bounds;
  const uint64_t ship_index = tile.ship_index;
  assert(ship_index < kUsedShip);

  v3f ship_position = kShip[ship_index].transform.position;
  tile_bounds.x = ship_position.x + (tile.cx * kTileWidth);
  tile_bounds.y = ship_position.y + (tile.cy * kTileHeight);
  tile_bounds.width = kTileWidth;
  tile_bounds.height = kTileHeight;

  return tile_bounds;
}

INLINE Tile*
ShipTile(uint64_t ship_index, uint16_t x, uint16_t y)
{
  return kShip[ship_index].map + (y * kShip[ship_index].map_width) + (x);
}

INLINE Tile*
ShipTile(Tile loc)
{
  Tile* tile = ShipTile(loc.ship_index, loc.cx, loc.cy);
  assert(*tile == loc);
  return tile;
}

void
TilemapResetShroud()
{
  for (int i = 0; i < kUsedShip; ++i) {
    Ship* ship = &kShip[i];
    Tile set_tile = *ShipTile(i, ship->map_width >> 1, ship->map_height >> 1);
    set_tile.flags = 0;
    set_tile.shroud = 1;
    BfsTileEnable(set_tile, kMapMaxWidth);
  }
}

Tile
ToShip(uint64_t ship_index, v3f world)
{
  Tile tile = kZeroTile;

  if (ship_index >= kUsedShip) return kZeroTile;
  if (!math::PointInRect(world.xy(), ShipBounds(ship_index))) return tile;

  v2f relpos = world.xy() - kShip[ship_index].transform.position.xy();
  v2i grid(relpos.x / kTileWidth, relpos.y / kTileHeight);
  return *ShipTile(ship_index, grid.x, grid.y);
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

}  // namespace simulation
