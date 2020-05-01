#pragma once

struct Cubef {
  Cubef() = default;
  Cubef(float x, float y, float z, float width, float height, float depth) :
    pos(x, y, z), width(width), height(height), depth(depth) {}
  Cubef(v3f pos, float width, float height, float depth) :
    pos(pos), width(width), height(height), depth(depth) {}
  Cubef(v3f pos, v3f bounds) :
    pos(pos), width(bounds.x), height(bounds.y), depth(bounds.z) {}
  v3f pos;
  float width;
  float height;
  float depth;
};


namespace math {


}  // namespace math
