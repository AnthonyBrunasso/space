#pragma once

namespace mood {

v2i
WorldToTile(v2f world)
{
  v2i t = v2i((s32)world.x / (s32)kTileWidth, (s32)world.y / (s32)kTileHeight);
  if (world.x < 0.f) t.x -= 1;
  if (world.y < 0.f) t.y -= 1;
  return t;
}

}
