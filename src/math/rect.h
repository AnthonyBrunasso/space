#pragma once

#include "vec.h"

namespace math {

struct AxisAlignedRect {
  v3f min; 
  v3f max; 
};

struct Rectf {
  Rectf() = default;
  Rectf(float x, float y, float width, float height) :
    x(x), y(y), width(width), height(height) {}
  float x;
  float y;
  float width;
  float height;
};

// Orients a rect so that it has positive widths and heights.
Rectf
OrientToAabb(const math::Rectf& rect)
{
  Rectf r = rect;
  if (rect.height < 0.f) {
    r.y += rect.height;
    r.height *= -1.f;
  }
  if (rect.width < 0.f) {
    r.x += rect.width;
    r.width *= -1.f;
  }
  return r;
}

v2f
RandomPointInRect(const math::Rectf& rect)
{
  float min_x = rect.x;
  float max_x = rect.x + rect.width;
  float min_y = rect.y;
  float max_y = rect.y + rect.height;
  return v2f(ScaleRange((float)rand() / RAND_MAX, 0.f, 1.f, min_x, max_x),
             ScaleRange((float)rand() / RAND_MAX, 0.f, 1.f, min_y, max_y));
}

}
