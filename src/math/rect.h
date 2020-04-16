#pragma once

#include "vec.h"

struct Rectf {
  Rectf() = default;
  Rectf(float x, float y, float width, float height)
      : x(x), y(y), width(width), height(height)
  {
  }
  float x;
  float y;
  float width;
  float height;

  v2f
  Center() const
  {
    return v2f(x + .5f * width, y + .5f * height);
  }

  v2f
  Min() const
  {
    return v2f(x, y);
  }

  v2f
  Max() const
  {
    return v2f(x + width, y + height);
  }
};

namespace math
{
// Orients a rect so that it has positive widths and heights.
Rectf
OrientToAabb(const Rectf& rect)
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
RandomPointInRect(const Rectf& rect)
{
  float min_x = rect.x;
  float max_x = rect.x + rect.width;
  float min_y = rect.y;
  float max_y = rect.y + rect.height;
  return v2f(ScaleRange((float)rand() / RAND_MAX, 0.f, 1.f, min_x, max_x),
             ScaleRange((float)rand() / RAND_MAX, 0.f, 1.f, min_y, max_y));
}

// The Anthony-especial algorithm for calculating a random point on exterior
// of a rectangle.
//
// 1. Root rect at origin.
// 2. Calculate a random point in the rect.
// 3. Project that point to bottom and left vectors made by rect.
// 4. Save vector with min projection distance.
// 5. Root rect s.t top right is now origin.
// 6. Calculate a random point in the rect.
// 7. Project that point to top and right vectors made by rect.
// 8. Save vector with min projection distance.
// 9. Return the min of distances from 4, 8 and retranslate to exterior.
v2f
RandomPointOnRect(const Rectf& rect)
{
  Rectf r = OrientToAabb(rect);
  v2f t(r.x, r.y);
  // Orient rect to origin.
  r.x -= r.x;
  r.y -= r.y;
  // Random point in rect rooted at origin.
  v2f pv = RandomPointInRect(r);
  // Project to left and bottom exteriors.
  v2f pl = Project(pv, v2f(0.f, r.height));
  v2f pb = Project(pv, v2f(r.width, 0.f));
  v2f pkeep =
      math::LengthSquared(pl - pv) < math::LengthSquared(pb - pv) ? pl : pb;
  // Orient rect s.t. top right is now origin.
  r.x -= r.width;
  r.y -= r.height;
  v2f nv = RandomPointInRect(r);
  // Project to top and right exteriors.
  v2f nt = Project(nv, v2f(-r.width, 0.f));
  v2f nr = Project(nv, v2f(0.f, -r.height));
  v2f nkeep =
      math::LengthSquared(nt - nv) < math::LengthSquared(nr - nv) ? nt : nr;
  // Return min distance vector. This part doesn't really matter. I'm sure you
  // could return the max or generate another random number between 0 and 3
  // and pick a random projection.
  if (math::LengthSquared(nkeep - nv) < math::LengthSquared(pkeep - pv)) {
    return nkeep + v2f(r.width, r.height) + t;
  }
  return pkeep + t;
}

bool
PointInRect(const v2f& point, const Rectf& rect)
{
  return (point.x >= rect.x && point.x <= rect.x + rect.width) &&
         (point.y >= rect.y && point.y <= rect.y + rect.height);
}

bool
IntersectRect(const Rectf& a, const Rectf& b)
{
  v2f amin = a.Min();
  v2f amax = a.Max();
  v2f bmin = b.Min();
  v2f bmax = b.Max();
  if (amin.x >= bmax.x || bmin.x >= amax.x) return false;
  if (amin.y >= bmax.y || bmin.y >= amax.y) return false;
  return true;
}

}  // namespace math
