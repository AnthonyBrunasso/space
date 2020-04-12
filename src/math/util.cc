#pragma once

#include "vec.h"

namespace math {

float
ScaleRange(float v, float smin, float smax, float tmin, float tmax)
{
  return (((v - smin) * (tmax - tmin)) / (smax - smin)) + tmin;
}

float
ScaleRange(float v, float smax, float tmax)
{
  return ((v) * (tmax)) / (smax);
}

v3f
Lerp(const v3f& a, const v3f& b, float t)
{
  printf("%.3f\n", t);
  return a * (1.f - t) + b * t;
}

}
