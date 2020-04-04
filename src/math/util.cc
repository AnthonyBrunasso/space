#pragma once

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

}