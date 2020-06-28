#pragma once

#include "vec.h"

#include <cstdlib>

namespace math {

r32
ScaleRange(r32 v, r32 smin, r32 smax, r32 tmin, r32 tmax)
{
  return (((v - smin) * (tmax - tmin)) / (smax - smin)) + tmin;
}

r32
ScaleRange(r32 v, r32 smax, r32 tmax)
{
  return ((v) * (tmax)) / (smax);
}

r32
Random(r32 min, r32 max)
{
  return math::ScaleRange((r32)rand() / RAND_MAX, 0.f, 1.f, min, max);
}

v3f
Lerp(const v3f& a, const v3f& b, r32 t)
{
  return a * (1.f - t) + b * t;
}

v4f
Lerp(const v4f& a, const v4f& b, r32 t)
{
  return a * (1.f - t) + b * t;
}

template <typename T>
T
Max(T x, T y)
{
  return x > y ? x : y;
}

template <typename T>
T
Min(T x, T y)
{
  return x < y ? x : y;
}

}
