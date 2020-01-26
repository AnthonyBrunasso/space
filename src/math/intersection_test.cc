#include <cassert>

#include "intersection.cc"
#include "vec.h"

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))
#endif

#define ASSERT_TRUE(x) assert(x)
#define ASSERT_FALSE(x) assert(!(x))

void
LineSegmentsIntersectPositive_One()
{
  v2f a_start(0.f, 1.f), a_end(1.f, 1.f);
  v2f b_start(0.5f, 0.f), b_end(0.5f, 1.5f);
  ASSERT_TRUE(math::LineSegmentsIntersect(a_start, a_end, b_start, b_end,
                                          nullptr, nullptr));
}

void
LineSegmentsIntersectPositive_Two()
{
  v2f a_start(-1.f, -1.5f), a_end(-3.f, -5.f);
  v2f b_start(-5.f, 0.f), b_end(0.f, -5.f);
  ASSERT_TRUE(math::LineSegmentsIntersect(a_start, a_end, b_start, b_end,
                                          nullptr, nullptr));
}

void
LineSegmentsIntersectPositive_Three()
{
  v2f a_start(3.f, -5.f), a_end(0.f, 0.f);
  v2f b_start(0.f, -4.f), b_end(3.f, 0.f);
  ASSERT_TRUE(math::LineSegmentsIntersect(a_start, a_end, b_start, b_end,
                                          nullptr, nullptr));
}

void
LineSegmentsIntersectNegative_One()
{
  v2f a_start(0.f, 1.f), a_end(1.f, 1.f);
  v2f b_start(0.5f, 0.f), b_end(0.5f, -1.5f);
  ASSERT_FALSE(math::LineSegmentsIntersect(a_start, a_end, b_start, b_end,
                                           nullptr, nullptr));
}

void
LineSegmentsIntersectNegative_Two()
{
  v2f a_start(-1.f, -1.5f), a_end(-3.f, -5.f);
  v2f b_start(-5.f, 0.f), b_end(0.f, 5.f);
  ASSERT_FALSE(math::LineSegmentsIntersect(a_start, a_end, b_start, b_end,
                                           nullptr, nullptr));
}

void
LineSegmentsIntersectNegative_Three()
{
  v2f a_start(3.f, -5.f), a_end(0.f, 0.f);
  v2f b_start(0.f, -4.f), b_end(-3.f, 0.f);
  ASSERT_FALSE(math::LineSegmentsIntersect(a_start, a_end, b_start, b_end,
                                           nullptr, nullptr));
}

void
PointInPolygon_One()
{
  v2f polygon[] = {
      {0.0f, 0.0f}, {10.0f, 0.0f}, {10.0f, 10.0f}, {0.0f, 10.0f}};
  v2f p(20.0f, 20.0f);
  ASSERT_FALSE(math::PointInPolygon(p, ARRAY_LENGTH(polygon), polygon));
}

void
PointInPolygon_Two()
{
  v2f polygon[] = {
      {0.0f, 0.0f}, {10.0f, 0.0f}, {10.0f, 10.0f}, {0.0f, 10.0f}};
  v2f p(5.f, 5.f);
  ASSERT_TRUE(math::PointInPolygon(p, ARRAY_LENGTH(polygon), polygon));
}

void
PointInPolygon_Three()
{
  v2f polygon[] = {{0.f, 0.f}, {5.f, 5.f}, {5.f, 0.f}};
  v2f p(3.f, 3.f);
  ASSERT_TRUE(math::PointInPolygon(p, ARRAY_LENGTH(polygon), polygon));
}

void
PointInPolygon_Four()
{
  v2f polygon[] = {{0.f, 0.f}, {5.f, 5.f}, {5.f, 0.f}};
  v2f p(5.f, 1.f);
  ASSERT_TRUE(math::PointInPolygon(p, ARRAY_LENGTH(polygon), polygon));
}

void
PointInPolygon_Five()
{
  v2f polygon[] = {{0.f, 0.f}, {5.f, 5.f}, {5.f, 0.f}};
  v2f p(8.f, 1.f);
  ASSERT_FALSE(math::PointInPolygon(p, ARRAY_LENGTH(polygon), polygon));
}

void
PointInPolygon_Six()
{
  v2f polygon[] = {{0.f, 0.f}, {10.f, 0.f}, {10.f, 10.f}, {0.f, 10.f}};
  v2f p(-1.f, 10.f);
  ASSERT_FALSE(math::PointInPolygon(p, ARRAY_LENGTH(polygon), polygon));
}

int
main(int argc, char** argv)
{
  LineSegmentsIntersectPositive_One();
  LineSegmentsIntersectPositive_Two();
  LineSegmentsIntersectPositive_Three();
  LineSegmentsIntersectNegative_One();
  LineSegmentsIntersectNegative_Two();
  LineSegmentsIntersectNegative_Three();
  PointInPolygon_One();
  PointInPolygon_Two();
  PointInPolygon_Three();
  PointInPolygon_Four();
  PointInPolygon_Five();
  PointInPolygon_Six();
  return 0;
}
