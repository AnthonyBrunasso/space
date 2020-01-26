#pragma once

#include "vec.h"

namespace math {

struct AxisAlignedRect {
  v3f min; 
  v3f max; 
};

struct Rectf {
  v2f min; 
  v2f max; 
};

struct Rect {
  Rect() = default;
  Rect(float x, float y, float width, float height) :
    x(x), y(y), width(width), height(height) {}
  float x;
  float y;
  float width;
  float height;
};

}
