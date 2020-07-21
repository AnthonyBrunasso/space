#pragma once

namespace mood {

v2i
WorldToTile(v2f world, r32 width = kTileWidth, r32 height = kTileHeight)
{
  v2i t = v2i((s32)world.x / (s32)width, (s32)world.y / (s32)height);
  if (world.x < 0.f) t.x -= 1;
  if (world.y < 0.f) t.y -= 1;
  return t;
}

}
