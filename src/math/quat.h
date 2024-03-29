#pragma once

#include <cmath>

#include "vec.h"


namespace math
{

template <typename T>
struct Quat {
  // TODO: This should probably normalize itself because it needs to
  // be a unit vector. Maybe do it before converting to rotation
  // matrix so it's only done when it needs to be.

  Quat(const T& w, const T& x, const T& y, const T& z) : w(w), x(x), y(y), z(z)
  {
  }

  Quat(r32 angle_degrees, const v3f& axis) { Set(angle_degrees, axis); }

  Quat() : Quat(0.0f, v3f(0.f, 0.f, 1.f)) { Set(angle_degrees, axis); }

  void
  Set(r32 new_angle, const v3f& new_axis)
  {
    angle_degrees = new_angle;
    axis = Normalize(new_axis);
    r32 angle_radians = (angle_degrees)*PI / 180.0f;
    //printf("DEGREES: %.2f\n", angle_degrees);
    w = cos(angle_radians / 2.0f);
    x = sin(angle_radians / 2.0f) * axis.x;
    y = sin(angle_radians / 2.0f) * axis.y;
    z = sin(angle_radians / 2.0f) * axis.z;
  }

  void
  Rotate(r32 angle_degrees_delta)
  {
    // Wrap the  angle?
    r32 angle = angle_degrees + angle_degrees_delta;
    Set(angle, axis);
  }

  v3f
  Forward() const
  {
    return v3f(-2.f * x * z + 2.f * w * y, -2.f * y * z - 2.f * w * x,
               -1.f + 2.f * x * x + 2.f * y * y);
  }

  v3f
  Up() const
  {
    return v3f(2.f * x * y + 2.f * w * z, 1.f - 2.f * x * x - 2.f * z * z,
               2.f * y * z - 2.f * w * x);
  }

  v3f
  Left() const
  {
    return v3f(-2.f * x * z + 2.f * w * y, -2.f * y * z - 2.f * w * x,
               -1.f + 2.f * x * x + 2.f * y * y);
  }

  T w;
  T x;
  T y;
  T z;

  v3f axis;
  r32 angle_degrees;
};

}  // namespace math

using Quatf = math::Quat<r32>;
