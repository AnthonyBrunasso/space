#pragma once

#include "vec.h"

namespace math {

struct AxisAlignedRect {
  math::Vec3f min; 
  math::Vec3f max; 
};

struct Rectf {
  math::Vec2f min; 
  math::Vec2f max; 
};

struct Rect {
  Rect(float x, float y, float width, float height) :
    x(x), y(y), width(width), height(height) {}
  float x;
  float y;
  float width;
  float height;
};

}
