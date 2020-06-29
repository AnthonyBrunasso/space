#pragma once

#include <cfloat>

namespace math
{
r32
Signed2DTriArea(const v2f& a, const v2f& b, const v2f& c)
{
  return (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x);
}

// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
b8
OnSegment(const v2f& p, const v2f& q, const v2f& r)
{
  if (q.x <= fmaxf(p.x, r.x) && q.x >= fminf(p.x, r.x) &&
      q.y <= fmaxf(p.y, r.y) && q.y >= fminf(p.y, r.y)) {
    return true;
  }
  return false;
}

b8
IsNear(r32 value, r32 target)
{
  return (target - value < FLT_EPSILON || value - target < FLT_EPSILON);
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
s32
Orientation(const v2f& p, const v2f& q, const v2f& r)
{
  r32 val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
  if (IsNear(val, 0.f)) return 0;  // colinear
  return (val > 0.f) ? 1 : 2;      // clock or counterclock wise
}

// The function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
b8
DoIntersect(const v2f& p1, const v2f& q1, const v2f& p2, const v2f& q2)
{
  // Find the four orientations needed for general and
  // special cases
  s32 o1 = Orientation(p1, q1, p2);
  s32 o2 = Orientation(p1, q1, q2);
  s32 o3 = Orientation(p2, q2, p1);
  s32 o4 = Orientation(p2, q2, q1);
  // General case
  if (o1 != o2 && o3 != o4) return true;
  // Special Cases
  // p1, q1 and p2 are colinear and p2 lies on segment p1q1
  if (o1 == 0 && OnSegment(p1, p2, q1)) return true;
  // p1, q1 and p2 are colinear and q2 lies on segment p1q1
  if (o2 == 0 && OnSegment(p1, q2, q1)) return true;
  // p2, q2 and p1 are colinear and p1 lies on segment p2q2
  if (o3 == 0 && OnSegment(p2, p1, q2)) return true;
  // p2, q2 and q1 are colinear and q1 lies on segment p2q2
  if (o4 == 0 && OnSegment(p2, q1, q2)) return true;
  return false;  // Doesn't fall in any of the above cases
}

// namespace

// pg 152. Real-Time Collision Detection by Christer Ericson
b8
IntersectLineSegment(const v2f& a_start, const v2f& a_end, const v2f& b_start,
                     const v2f& b_end, r32* time, v2f* position)
{
  r32 a1 = Signed2DTriArea(a_start, a_end, b_end);
  r32 a2 = Signed2DTriArea(a_start, a_end, b_start);
  if (a1 * a2 >= 0.f) return false;
  r32 a3 = Signed2DTriArea(b_start, b_end, a_start);
  r32 a4 = a3 + a2 - a1;
  if (a3 * a4 >= 0.f) return false;
  if (time && position) {
    *time = a3 / (a3 - a4);
    *position = a_start + (a_end - a_start) * *time;
  }
  return true;
}

// https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/
b8
PointInPolygon(const v2f& point, const u64 polygon_size, v2f* polygon)
{
  // There must be at least 3 vertices in polygon[]
  if (polygon_size < 3) return false;
  // Create a point for line segment from p to infinite
  v2f extreme(FLT_MAX, point.y);
  // Count intersections of the above line with sides of polygon
  s32 count = 0, i = 0;
  do {
    s32 next = (i + 1) % polygon_size;
    // Check if the line segment from 'p' to 'extreme' intersects
    // with the line segment from 'polygon[i]' to 'polygon[next]'
    if (DoIntersect(polygon[i], polygon[next], point, extreme)) {
      // If the point 'p' is colinear with line segment 'i-next',
      // then check if it lies on segment. If it lies, return true,
      // otherwise false
      if (Orientation(polygon[i], point, polygon[next]) == 0) {
        return OnSegment(polygon[i], point, polygon[next]);
      }
      count++;
    }
    i = next;
  } while (i != 0);
  // Return true if count is odd, false otherwise
  return count & 1;  // Same as (count%2 == 1)
}

b8
PointInCircle(const v2f& point, const v2f& center, r32 radius)
{
  return LengthSquared(point - center) < radius * radius;
}

}  // namespace math
