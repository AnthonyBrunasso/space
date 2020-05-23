#pragma once

#include "mat.h"
#include "quat.h"
#include "vec.h"

namespace math
{
#define ONE_DEG_IN_RAD (2.0 * PI) / 360.0  // 0.017444444

Mat4f
Inverse(const Mat4f& m)
{
  Mat4f inv;
  inv.data_[0] = m.data_[5]  * m.data_[10] * m.data_[15] - 
           m.data_[5]  * m.data_[11] * m.data_[14] - 
           m.data_[9]  * m.data_[6]  * m.data_[15] + 
           m.data_[9]  * m.data_[7]  * m.data_[14] +
           m.data_[13] * m.data_[6]  * m.data_[11] - 
           m.data_[13] * m.data_[7]  * m.data_[10];

  inv.data_[4] = -m.data_[4]  * m.data_[10] * m.data_[15] + 
            m.data_[4]  * m.data_[11] * m.data_[14] + 
            m.data_[8]  * m.data_[6]  * m.data_[15] - 
            m.data_[8]  * m.data_[7]  * m.data_[14] - 
            m.data_[12] * m.data_[6]  * m.data_[11] + 
            m.data_[12] * m.data_[7]  * m.data_[10];

  inv.data_[8] = m.data_[4]  * m.data_[9] * m.data_[15] - 
           m.data_[4]  * m.data_[11] * m.data_[13] - 
           m.data_[8]  * m.data_[5] * m.data_[15] + 
           m.data_[8]  * m.data_[7] * m.data_[13] + 
           m.data_[12] * m.data_[5] * m.data_[11] - 
           m.data_[12] * m.data_[7] * m.data_[9];

  inv.data_[12] = -m.data_[4]  * m.data_[9] * m.data_[14] + 
             m.data_[4]  * m.data_[10] * m.data_[13] +
             m.data_[8]  * m.data_[5] * m.data_[14] - 
             m.data_[8]  * m.data_[6] * m.data_[13] - 
             m.data_[12] * m.data_[5] * m.data_[10] + 
             m.data_[12] * m.data_[6] * m.data_[9];

  inv.data_[1] = -m.data_[1]  * m.data_[10] * m.data_[15] + 
            m.data_[1]  * m.data_[11] * m.data_[14] + 
            m.data_[9]  * m.data_[2] * m.data_[15] - 
            m.data_[9]  * m.data_[3] * m.data_[14] - 
            m.data_[13] * m.data_[2] * m.data_[11] + 
            m.data_[13] * m.data_[3] * m.data_[10];

  inv.data_[5] = m.data_[0]  * m.data_[10] * m.data_[15] - 
           m.data_[0]  * m.data_[11] * m.data_[14] - 
           m.data_[8]  * m.data_[2] * m.data_[15] + 
           m.data_[8]  * m.data_[3] * m.data_[14] + 
           m.data_[12] * m.data_[2] * m.data_[11] - 
           m.data_[12] * m.data_[3] * m.data_[10];

  inv.data_[9] = -m.data_[0]  * m.data_[9] * m.data_[15] + 
            m.data_[0]  * m.data_[11] * m.data_[13] + 
            m.data_[8]  * m.data_[1] * m.data_[15] - 
            m.data_[8]  * m.data_[3] * m.data_[13] - 
            m.data_[12] * m.data_[1] * m.data_[11] + 
            m.data_[12] * m.data_[3] * m.data_[9];

  inv.data_[13] = m.data_[0]  * m.data_[9] * m.data_[14] - 
            m.data_[0]  * m.data_[10] * m.data_[13] - 
            m.data_[8]  * m.data_[1] * m.data_[14] + 
            m.data_[8]  * m.data_[2] * m.data_[13] + 
            m.data_[12] * m.data_[1] * m.data_[10] - 
            m.data_[12] * m.data_[2] * m.data_[9];

  inv.data_[2] = m.data_[1]  * m.data_[6] * m.data_[15] - 
           m.data_[1]  * m.data_[7] * m.data_[14] - 
           m.data_[5]  * m.data_[2] * m.data_[15] + 
           m.data_[5]  * m.data_[3] * m.data_[14] + 
           m.data_[13] * m.data_[2] * m.data_[7] - 
           m.data_[13] * m.data_[3] * m.data_[6];

  inv.data_[6] = -m.data_[0]  * m.data_[6] * m.data_[15] + 
            m.data_[0]  * m.data_[7] * m.data_[14] + 
            m.data_[4]  * m.data_[2] * m.data_[15] - 
            m.data_[4]  * m.data_[3] * m.data_[14] - 
            m.data_[12] * m.data_[2] * m.data_[7] + 
            m.data_[12] * m.data_[3] * m.data_[6];

  inv.data_[10] = m.data_[0]  * m.data_[5] * m.data_[15] - 
            m.data_[0]  * m.data_[7] * m.data_[13] - 
            m.data_[4]  * m.data_[1] * m.data_[15] + 
            m.data_[4]  * m.data_[3] * m.data_[13] + 
            m.data_[12] * m.data_[1] * m.data_[7] - 
            m.data_[12] * m.data_[3] * m.data_[5];

  inv.data_[14] = -m.data_[0]  * m.data_[5] * m.data_[14] + 
             m.data_[0]  * m.data_[6] * m.data_[13] + 
             m.data_[4]  * m.data_[1] * m.data_[14] - 
             m.data_[4]  * m.data_[2] * m.data_[13] - 
             m.data_[12] * m.data_[1] * m.data_[6] + 
             m.data_[12] * m.data_[2] * m.data_[5];

  inv.data_[3] = -m.data_[1] * m.data_[6] * m.data_[11] + 
            m.data_[1] * m.data_[7] * m.data_[10] + 
            m.data_[5] * m.data_[2] * m.data_[11] - 
            m.data_[5] * m.data_[3] * m.data_[10] - 
            m.data_[9] * m.data_[2] * m.data_[7] + 
            m.data_[9] * m.data_[3] * m.data_[6];

  inv.data_[7] = m.data_[0] * m.data_[6] * m.data_[11] - 
           m.data_[0] * m.data_[7] * m.data_[10] - 
           m.data_[4] * m.data_[2] * m.data_[11] + 
           m.data_[4] * m.data_[3] * m.data_[10] + 
           m.data_[8] * m.data_[2] * m.data_[7] - 
           m.data_[8] * m.data_[3] * m.data_[6];

  inv.data_[11] = -m.data_[0] * m.data_[5] * m.data_[11] + 
             m.data_[0] * m.data_[7] * m.data_[9] + 
             m.data_[4] * m.data_[1] * m.data_[11] - 
             m.data_[4] * m.data_[3] * m.data_[9] - 
             m.data_[8] * m.data_[1] * m.data_[7] + 
             m.data_[8] * m.data_[3] * m.data_[5];

  inv.data_[15] = m.data_[0] * m.data_[5] * m.data_[10] - 
            m.data_[0] * m.data_[6] * m.data_[9] - 
            m.data_[4] * m.data_[1] * m.data_[10] + 
            m.data_[4] * m.data_[2] * m.data_[9] + 
            m.data_[8] * m.data_[1] * m.data_[6] - 
            m.data_[8] * m.data_[2] * m.data_[5];

  r32 det = m.data_[0] * inv.data_[0] + m.data_[1] * inv.data_[4] + m.data_[2] * inv.data_[8] + m.data_[3] * inv.data_[12];

  det = 1.0f / det;

  for (int i = 0; i < 16; i++)
      inv.data_[i] = inv.data_[i] * det;

  return inv;
}

Mat4f
Identity()
{
return Mat4f(1.0f, 0.0f, 0.0f, 0.0f,
             0.0f, 1.0f, 0.0f, 0.0f,
             0.0f, 0.0f, 1.0f, 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f);
}

Mat4f
Translation(const v3f& translation)
{
return Mat4f(1.0f, 0.0f, 0.0f, 0.0f,
             0.0f, 1.0f, 0.0f, 0.0f,
             0.0f, 0.0f, 1.0f, 0.0f,
             translation.x, translation.y, translation.z, 1.0f);
}

Mat4f
Scale(const v3f& scale)
{
  return Mat4f(scale.x, 0.0f,    0.0f, 0.0f,
               0.0f, scale.y,    0.0f, 0.0f,
               0.0f,    0.0f, scale.z, 0.0f,
               0.0f,    0.0f,    0.0f, 1.0f);
}

Mat4f
LookAt(const v3f& eye, const v3f& target, const v3f& up)
{
  v3f f = math::Normalize(eye - target);
  v3f r = math::Normalize(math::Cross(up, f));
  v3f u = math::Cross(f, r);

#if 0
  // Keeping this around to remind myself how the below is derived.
  // NOTE - See the transposed columns, this is not a mistake. The view
  // matrix is the inverse of the camera transform. The camera transform
  // is given by the above basis and to invert a rotation matrix (orthonormal
  // basis) we can transpose it.
  Mat4f orientation(
      r.x, u.x, f.x, 0.f,
      r.y, u.y, f.y, 0.f,
      r.z, u.z, f.z, 0.f,
      0.f, 0.f, 0.f, 1.f);
  return (orientation * Translation(-eye));
#else
   return Mat4f(
      r.x, u.x, f.x, 0.f,
      r.y, u.y, f.y, 0.f,
      r.z, u.z, f.z, 0.f,
      math::Dot(r, -eye), math::Dot(u, -eye), math::Dot(f, -eye), 1.f);
#endif
}

Mat4f
Perspective(r32 fov_degrees, r32 aspect, r32 znear, r32 zfar)
{
  r32 fov = fov_degrees * ONE_DEG_IN_RAD;
  r32 thf = tan(fov / 2.f);
  return Mat4f(
      1.f / (aspect * thf), 0.f, 0.f, 0.f,
      0.f, 1.f / thf, 0.f, 0.f,
      0.f, 0.f, -(zfar + znear) / (zfar - znear), -1.f,
      0.f, 0.0f, -(2.f * zfar * znear) / (zfar - znear), 0.0f);
}

Mat4f
Ortho(r32 right, r32 left, r32 top, r32 bottom, r32 far_clip, r32 near_clip)
{
  // Goal with this matrix is to scale a point, in likely screen space relative
  // to the cameras to GL space or the unit cube.
  //
  // To do that use the diagonal of this matrix to to scale the point
  // down to a unit cube.
  r32 w = right - left;
  w = w == 0.f ? 1.f : w;
  r32 h = top - bottom;
  h = h == 0.f ? 1.f : h;
  r32 d = far_clip - near_clip;
  d = d == 0.f ? 1.f : d;
  return Mat4f(2.f / w, 0.f    , 0.f     , 0.f,
               0.f    , 2.f / h, 0.f     , 0.f,
               0.f    , 0.f    , 2.f / d, -1.f,
               0.f    , 0.f    , 0.f     , 1.f);
}

// This function orients origin to bottom left of screen. Useful for UI so
// points can be specified in actual screen space.
Mat4f
Ortho2(r32 right, r32 left, r32 top, r32 bottom, r32 far_clip, r32 near_clip)
{
  r32 w = right - left;
  w = w == 0.f ? 1.f : w;
  r32 h = top - bottom;
  h = h == 0.f ? 1.f : h;
  r32 d = far_clip - near_clip;
  d = d == 0.f ? 1.f : d;
  return Mat4f(2.f / w, 0.f    , 0.f     , 0.f,
               0.f    , 2.f / h, 0.f     , 0.f,
               0.f    , 0.f    , -2.f / d, 0.f,
               -(right + left) / w,
               -(top + bottom) / h,
               -(near_clip + far_clip) / d,
               1.f);
}

Mat4f
RotationX(r32 angle_degrees)
{
  if (angle_degrees == 0.f) return math::Identity();
  r32 angle_radians = (angle_degrees)*PI / 180.0f;
  r32 c = cos(angle_radians);
  r32 s = sin(angle_radians);
  return Mat4f(1.0f, 0.f, 0.f, 0.f, 
               0.0f, c  , -s , 0.f,
               0.0f, s  ,  c , 0.f,
               0.0f, 0.f, 0.f, 1.f);
}

Mat4f
RotationY(r32 angle_degrees)
{
  if (angle_degrees == 0.f) return math::Identity();
  r32 angle_radians = (angle_degrees)*PI / 180.0f;
  r32 c = cos(angle_radians);
  r32 s = sin(angle_radians);
  return Mat4f(c   , 0.0f, s   , 0.f, 
               0.0f, 1.0f, 0.0f, 0.f,
               -s  , 0.0f, c   , 0.f,
               0.0f, 0.f , 0.f , 1.f);
}

Mat4f
RotationZ(r32 angle_degrees)
{
  if (angle_degrees == 0.f) return math::Identity();
  r32 angle_radians = (angle_degrees)*PI / 180.0f;
  r32 c = cos(angle_radians);
  r32 s = sin(angle_radians);
  return Mat4f(c  , -s , 0.f, 0.f,
               s  , c  , 0.f, 0.f,
               0.f, 0.f, 1.f, 0.f,
               0.f, 0.f, 0.f, 1.f);
}

Mat4f
Model(const v3f& position, const v3f& scale, const Quatf& quat) {
  Mat4f model;
  model.data_[0] = scale.x * (1.f - 2.f * quat.y * quat.y - 2.f * quat.z * quat.z);
  model.data_[1] = scale.y * (2.f * quat.x * quat.y - 2.f * quat.w * quat.z);
  model.data_[2] = scale.z * (2.f * quat.x * quat.z + 2 * quat.w * quat.y);
  model.data_[3] = 0.0f;
  model.data_[4] = scale.x * (2.f * quat.x * quat.y + 2.f * quat.w * quat.z);
  model.data_[5] = scale.y * (1.f - 2.f * quat.x * quat.x - 2.f * quat.z * quat.z);
  model.data_[6] = scale.z * (2.f * quat.y * quat.z - 2.f * quat.w * quat.x);
  model.data_[7] = 0.0f;
  model.data_[8] = scale.x * (2.f * quat.x * quat.z - 2.f * quat.w * quat.y);
  model.data_[9] = scale.y * (2.f * quat.y * quat.z + 2.f * quat.w * quat.x);
  model.data_[10] = scale.z * (1.f - 2.f * quat.x * quat.x - 2.f * quat.y * quat.y);
  model.data_[11] = 0.0f;
  model.data_[12] = position.x;
  model.data_[13] = position.y;
  model.data_[14] = position.z;
  model.data_[15] = 1.0f;
  return model;
}

Mat4f
Model(const v3f& position, const v3f& scale) {
  Mat4f model;
  model.data_[0] = scale.x;
  model.data_[1] = 0.0f;
  model.data_[2] = 0.0f;
  model.data_[3] = 0.0f;
  model.data_[4] = 0.0f;
  model.data_[5] = scale.y;
  model.data_[6] = 0.0f;
  model.data_[7] = 0.0f;
  model.data_[8] = 0.0f;
  model.data_[9] = 0.0f;
  model.data_[10] = scale.z;
  model.data_[11] = 0.0f;
  model.data_[12] = position.x;
  model.data_[13] = position.y;
  model.data_[14] = position.z;
  model.data_[15] = 1.0f;
  return model;
}

v3f
Unproject(const v3f& screen, const Mat4f& model, const Mat4f& proj,
          v2f viewport)
{
  Mat4f inv = Inverse(proj * model);
  v4f tmp(screen.x, screen.y, screen.z, 1.f);
  tmp.x = (tmp.x) / (viewport.x);
  tmp.y = (tmp.y) / (viewport.y);
  tmp.x = tmp.x * 2.f - 1.f;
  tmp.y = tmp.y * 2.f - 1.f;
  v4f obj = inv * tmp;
  obj /= obj.w;
  return obj.xyz();
}

}  // namespace math
