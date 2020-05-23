#pragma once

#include "vec.h"

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

}
