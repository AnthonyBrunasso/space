#pragma once

#include <cassert>

namespace math {

template <u32 N>
struct Polygon {
  v2f
  Center() const
  {
    v2f sum = {};
    for (u32 i = 0; i < N; ++i) {
      sum += vertex[i];
    }
    return sum / N;
  }

  v2f
  Vertex(u32 i) const
  {
    assert(i < N);
    return vertex[i];
  }

  u32 size() const { return N; }
  v2f vertex[N];
};

template <u32 N1, u32 N2>
bool
IntersectPolygon(const Polygon<N1>& p1, const Polygon<N2>& p2,
                 v2f* collision_start, v2f* collision_end)
{
  // Check of line segments from p1 center intersect with edges of p2.
  v2f p1_center = p1.Center();
  for (u32 i = 0; i < p1.size(); ++i) {
    v2f diff = p1.Vertex(i) - p1_center;
    v2f p1_end = p1_center + diff;
    for (u32 j = 0; j < p2.size(); ++j) {
      v2f p2_start = p2.Vertex(j);
      v2f p2_end = p2.Vertex((j + 1) % N2);
      r32 time;
      v2f position;
      if (math::IntersectLineSegment(
            p1_center, p1_end, p2_start, p2_end, &time, &position)) {
        *collision_start = position;
        *collision_end = p1_end;
        return true;
      }
    }
  }

  v2f p2_center = p2.Center();
  for (u32 i = 0; i < p2.size(); ++i) {
    v2f diff = p2.Vertex(i) - p2_center;
    v2f p2_end = p2_center + diff;
    for (u32 j = 0; j < p1.size(); ++j) {
      v2f p1_start = p1.Vertex(j);
      v2f p1_end = p1.Vertex((j + 1) % N2);
      r32 time;
      v2f position;
      if (math::IntersectLineSegment(
            p2_center, p2_end, p1_start, p1_end, &time, &position)) {
        *collision_start = position;
        *collision_end = p2_end;
        return true;
      }
    }
  }

  return false;
}

}
