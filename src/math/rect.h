#pragma once

#include <cassert>
#include <cstdlib>

#include "common/macro.h"
#include "math/polygon.cc"

#include "vec.h"
#include "util.cc"

struct Rectf {
  Rectf() = default;
  Rectf(const v2f& min, const v2f& dims)
      : x(min.x), y(min.y), width(dims.x), height(dims.y) {
  }

  Rectf(const v2f& min, r32 width, r32 height)
      : x(min.x), y(min.y), width(width), height(height) {
  }

  Rectf(r32 x, r32 y, r32 width, r32 height)
      : x(x), y(y), width(width), height(height) {
  }

  r32 x;
  r32 y;
  r32 width;
  r32 height;

  v2f Dims() const {
    return v2f(width, height);
  }

  v2f Center() const {
    return v2f(x + .5f * width, y + .5f * height);
  }

  v2f TopLeft() const {
    return Min() + v2f(0.f, height);
  }

  v2f TopRight() const {
    return Max();
  }

  v2f BottomLeft() const {
    return Min();
  }

  v2f BottomRight() const {
    return Min() + v2f(width, 0.f);
  }

  v2f Min() const {
    return v2f(x, y);
  }

  v2f Max() const {
    return v2f(x + width, y + height);
  }

  void SetMin(v2f min) {
    x = min.x;
    y = min.y;
  }

  // Returns the nearest edge from pt
  v2f NearestEdge(v2f pt) {
    r32 dist[4] = {
      math::LengthSquared(pt - BottomLeft()),
      math::LengthSquared(pt - TopLeft()),
      math::LengthSquared(pt - TopRight()),
      math::LengthSquared(pt - BottomRight())};
    s32 min_idx = 0;
    r32 min_val = dist[0];
    for (s32 i = 1; i < 4; ++i) {
      if (dist[i] < min_val) {
        min_idx = i;
        min_val = dist[i];
      }
    }
    assert(min_idx >= 0 && min_idx <= 3);
    if (min_idx == 0) return BottomLeft();
    if (min_idx == 1) return TopLeft();
    if (min_idx == 2) return TopRight();
    return BottomRight();
  }

  math::Polygon<4> Polygon() const {
    math::Polygon<4> poly;
    poly.vertex[0] = v2f(x, y);
    poly.vertex[1] = v2f(x, y + height);
    poly.vertex[2] = v2f(x + width, y + height);
    poly.vertex[3] = v2f(x + width, y);
    return poly;
  }

  math::Polygon<4> Rotate(r32 rotation) const {
    // Avoid cos / sin.
    if (rotation == 0.f) return Polygon();
    math::Polygon<4> poly;
    v2f center = Center();
    r32 angle = rotation * PI / 180.0f;
    r32 cos_a = cos(angle);
    r32 sin_a = sin(angle);
    poly.vertex[0] = math::Rotate(Min() - center, cos_a, sin_a);
    poly.vertex[1] = math::Rotate(v2f(x, y + height) - center, cos_a, sin_a);
    poly.vertex[2] = math::Rotate(Max() - center, cos_a, sin_a);
    poly.vertex[3] = math::Rotate(v2f(x + width, y) - center, cos_a, sin_a);
    poly.vertex[0] += center;
    poly.vertex[1] += center;
    poly.vertex[2] += center;
    poly.vertex[3] += center;
    return poly;
  }

  Rectf RightHalf() {
    return Rectf(x + width / 2.f, y, width / 2.f, height);
  }

  Rectf LeftHalf() {
    return Rectf(x, y, width / 2.f, height);
  }

  void DebugPrint() {
    printf("Rectf(x:%.2f, y:%.2f, width:%.2f, height:%.2f)\n", x, y, width, height);
  }
};

namespace math
{

Rectf MakeRect(v2f min, v2f max) {
  return Rectf(min.x, min.y, max.x - min.x, max.y - min.y);
}

void PrintRect(const Rectf& rect) {
  printf("Rect(%.2f,%.2f,%.2f,%.2f)\n",
         rect.x, rect.y, rect.width, rect.height);
}

// Orients a rect so that it has positive widths and heights.
Rectf OrientToAabb(const Rectf& rect) {
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

v2f RandomPointInRect(const Rectf& rect) {
  r32 min_x = rect.x;
  r32 max_x = rect.x + rect.width;
  r32 min_y = rect.y;
  r32 max_y = rect.y + rect.height;
  return v2f(ScaleRange((r32)rand() / RAND_MAX, 0.f, 1.f, min_x, max_x),
             ScaleRange((r32)rand() / RAND_MAX, 0.f, 1.f, min_y, max_y));
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
v2f RandomPointOnRect(const Rectf& rect) {
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

b8 PointInRect(const v2f& point, const Rectf& rect) {
  return (point.x >= rect.x && point.x <= rect.x + rect.width) &&
         (point.y >= rect.y && point.y <= rect.y + rect.height);
}

b8 IntersectRect(const Rectf& a, const Rectf& b, Rectf* intersection = nullptr) {
  v2f amin = a.Min();
  v2f amax = a.Max();
  v2f bmin = b.Min();
  v2f bmax = b.Max();
  if (amin.x >= bmax.x || bmin.x >= amax.x) return false;
  if (amin.y >= bmax.y || bmin.y >= amax.y) return false;
  if (intersection) {
    r32 x = Max(a.x, b.x);
    r32 y = Max(a.y, b.y);
    r32 w = Min(amax.x, bmax.x) - x;
    r32 h = Min(amax.y, bmax.y) - y;
    *intersection = Rectf(x, y, w, h);
  }
  return true;
}

// Check if a is full contained in b.
b8 IsContainedInRect(const Rectf& a, const Rectf& b) {
  v2f amin = a.Min();
  v2f amax = a.Max();
  v2f bmin = b.Min();
  v2f bmax = b.Max();
  if (amin.x < bmin.x || amax.x > bmax.x) return false;
  if (amin.y < bmin.y || amax.y > bmax.y) return false;
  return true;
}

r32 DistanceBetween(const Rectf& a, const Rectf& b) {
  v2f atl = a.Min() + v2f(0.f, a.height);
  v2f abl = a.Min();
  v2f atr = a.Min() + v2f(a.width, a.height);
  v2f abr = a.Min() + v2f(a.width, 0.f);

  v2f btl = b.Min() + v2f(0.f, b.height);
  v2f bbl = b.Min();
  v2f btr = b.Min() + v2f(b.width, b.height);
  v2f bbr = b.Min() + v2f(b.width, 0.f);

  if (abr.x < btl.x && abr.y > btl.y) {
    return Length(abr - btl);
  }

  if (abl.x > btr.x && abl.y > btr.y) {
    return Length(abl - btr);
  }

  if (atr.x < bbl.x && atr.y < bbl.y) {
    return Length(atr - bbl);
  }

  if (atl.x > bbr.x && atl.y < bbr.y) {
    return Length(atl - bbr);
  }

  if (abl.x > bbr.x) {
    return abl.x - bbr.x;
  }

  if (abr.x < bbl.x) {
    return bbl.x - abr.x;
  }

  if (abr.y > btr.y) {
    return abr.y - btr.y;
  }

  if (atr.y < bbr.y) {
    return bbr.y - atr.y;
  }

  return 0.f;
}

}  // namespace math
