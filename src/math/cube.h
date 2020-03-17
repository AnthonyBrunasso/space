#pragma once

namespace math {

struct Cubef {
  Cubef() = default;
  Cubef(v3f pos, float width, float height, float depth) :
    pos(pos), width(width), height(height), depth(depth) {}
  v3f pos;
  float width;
  float height;
  float depth;
};

}  // namespace math
