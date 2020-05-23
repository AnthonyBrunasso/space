#pragma once

struct Cubef {
  Cubef() = default;
  Cubef(r32 x, r32 y, r32 z, r32 width, r32 height, r32 depth) :
    pos(x, y, z), width(width), height(height), depth(depth) {}
  Cubef(v3f pos, r32 width, r32 height, r32 depth) :
    pos(pos), width(width), height(height), depth(depth) {}
  Cubef(v3f pos, v3f bounds) :
    pos(pos), width(bounds.x), height(bounds.y), depth(bounds.z) {}
  v3f pos;
  r32 width;
  r32 height;
  r32 depth;
};
