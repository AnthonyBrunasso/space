#pragma once

#include "mat.h"
#include "quat.h"
#include "vec.h"

namespace math
{
#define ONE_DEG_IN_RAD (2.0 * PI) / 360.0  // 0.017444444

inline
Mat4f
CreateIdentityMatrix()
{
  return Mat4f(1.0f, 0.0f, 0.0f, 0.0f,
               0.0f, 1.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 1.0f, 0.0f,
               0.0f, 0.0f, 0.0f, 1.0f);
}

inline
Mat4f
CreateTranslationMatrix(const Vec3f& translation)
{
  return Mat4f(1.0f, 0.0f, 0.0f, 0.0f,
               0.0f, 1.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 1.0f, 0.0f,
               translation.x, translation.y, translation.z, 1.0f);
}

inline
Mat4f
CreateScaleMatrix(const Vec3f& scale)
{
  return Mat4f(scale.x, 0.0f, 0.0f, 0.0f,
               0.0f, scale.y, 0.0f, 0.0f,
               0.0f, 0.0f, scale.z, 0.0f,
               0.0f, 0.0f, 0.0f, 1.0f);
}

inline
Mat4f
CreateRotationMatrix(const Quatf& quat)
{
  Mat4f rotation;
  rotation[0] = 1.f - 2.f * quat.y * quat.y - 2.f * quat.z * quat.z;
  rotation[1] = 2.f * quat.x * quat.y - 2.f * quat.w * quat.z;
  rotation[2] = 2.f * quat.x * quat.z + 2 * quat.w * quat.y;
  rotation[3] = 0.0f;
  rotation[4] = 2.f * quat.x * quat.y + 2.f * quat.w * quat.z;
  rotation[5] = 1.f - 2.f * quat.x * quat.x - 2.f * quat.z * quat.z;
  rotation[6] = 2.f * quat.y * quat.z - 2.f * quat.w * quat.x;
  rotation[7] = 0.0f;
  rotation[8] = 2.f * quat.x * quat.z - 2.f * quat.w * quat.y;
  rotation[9] = 2.f * quat.y * quat.z + 2.f * quat.w * quat.x;
  rotation[10] = 1.f - 2.f * quat.x * quat.x - 2.f * quat.y * quat.y;
  rotation[11] = 0.0f;
  rotation[12] = 0.0f;
  rotation[13] = 0.0f;
  rotation[14] = 0.0f;
  rotation[15] = 1.0f;
  return rotation;
}

inline
Mat4f
CreateViewMatrix(const Vec3f& translation, const Quatf& quat)
{
  auto mat = CreateRotationMatrix(quat).Transpose();
  Mat4f view;
  view[0] = mat[0];
  view[1] = mat[1];
  view[2] = mat[2];
  view[3] = 0.0f;
  view[4] = mat[4];
  view[5] = mat[5];
  view[6] = mat[6];
  view[7] = 0.0f;
  view[8] = mat[8];
  view[9] = mat[9];
  view[10] = mat[10];
  view[11] = 0.0f;
  view[12] = -translation.x;
  view[13] = -translation.y;
  view[14] = -translation.z;
  view[15] = 1.0f;
  return view;
}

inline
Mat4f
CreatePerspectiveMatrix(float width, float height, float near_clip, float far_clip, 
                        float fov_degrees)
{
  float fov = fov_degrees * ONE_DEG_IN_RAD;
  float aspect = width / height;
  // matrix components
  float range = tan(fov * 0.5f) * near_clip;
  float Sx = (2.0f * near_clip) / (range * aspect + range * aspect);
  float Sy = near_clip / range;
  float Sz = -(far_clip + near_clip) / (far_clip - near_clip);
  float Pz = -(2.0f * far_clip * near_clip) / (far_clip - near_clip);
  return Mat4f(  Sx, 0.0f, 0.0f,  0.0f,
                 0.0f,   Sy, 0.0f,  0.0f,
                 0.0f, 0.0f,   Sz, -1.0f,
                 0.0f, 0.0f,   Pz,  0.0f);
}

inline
Mat4f
CreateOrthographicMatrix(float right, float left, float top, float bottom,
                         float far_clip, float near_clip)
{
  // Goal with this matrix is to scale a point, in likely screen space relative
  // to the cameras to GL space or the unit cube.
  //
  // To do that use the diagonal of this matrix to to scale the point
  // down to a unit cube.
  float w = right - left;
  w = w == 0.f ? 1.f : w;
  float h = top - bottom;
  h = h == 0.f ? 1.f : h;
  float d = far_clip - near_clip;
  d = d == 0.f ? 1.f : d;
  return Mat4f(2.f / w, 0.f    , 0.f     , 0.f,
               0.f    , 2.f / h, 0.f     , 0.f,
               0.f    , 0.f    , -2.f / d, 0.f,
               0.f    , 0.f    , 0.f     , 1.f);
}

// This function orients origin to bottom left of screen. Useful for UI so
// points can be specified in actual screen space.
inline
Mat4f
CreateOrthographicMatrix2(float right, float left, float top, float bottom,
                         float far_clip, float near_clip)
{
  float w = right - left;
  w = w == 0.f ? 1.f : w;
  float h = top - bottom;
  h = h == 0.f ? 1.f : h;
  float d = far_clip - near_clip;
  d = d == 0.f ? 1.f : d;
  return Mat4f(2.f / w, 0.f    , 0.f     , 0.f,
               0.f    , 2.f / h, 0.f     , 0.f,
               0.f    , 0.f    , -2.f / d, 0.f,
               -(right + left) / w,
               -(top + bottom) / h,
               -(near_clip + far_clip) / d,
               1.f);
}

inline
Mat4f
CreateModelMatrix(const math::Vec3f& position, const math::Vec3f& scale,
                  const math::Quatf& quat) {
  Mat4f model;
  model[0] = scale.x * (1.f - 2.f * quat.y * quat.y - 2.f * quat.z * quat.z);
  model[1] = scale.y * (2.f * quat.x * quat.y - 2.f * quat.w * quat.z);
  model[2] = scale.z * (2.f * quat.x * quat.z + 2 * quat.w * quat.y);
  model[3] = 0.0f;
  model[4] = scale.x * (2.f * quat.x * quat.y + 2.f * quat.w * quat.z);
  model[5] = scale.y * (1.f - 2.f * quat.x * quat.x - 2.f * quat.z * quat.z);
  model[6] = scale.z * (2.f * quat.y * quat.z - 2.f * quat.w * quat.x);
  model[7] = 0.0f;
  model[8] = scale.x * (2.f * quat.x * quat.z - 2.f * quat.w * quat.y);
  model[9] = scale.y * (2.f * quat.y * quat.z + 2.f * quat.w * quat.x);
  model[10] = scale.z * (1.f - 2.f * quat.x * quat.x - 2.f * quat.y * quat.y);
  model[11] = 0.0f;
  model[12] = position.x;
  model[13] = position.y;
  model[14] = position.z;
  model[15] = 1.0f;
  return model;
}

inline
Mat4f
CreateModelMatrix(const math::Vec3f& position, const math::Vec3f& scale) {
  Mat4f model;
  model[0] = scale.x;
  model[1] = 0.0f;
  model[2] = 0.0f;
  model[3] = 0.0f;
  model[4] = 0.0f;
  model[5] = scale.y;
  model[6] = 0.0f;
  model[7] = 0.0f;
  model[8] = 0.0f;
  model[9] = 0.0f;
  model[10] = scale.z;
  model[11] = 0.0f;
  model[12] = position.x;
  model[13] = position.y;
  model[14] = position.z;
  model[15] = 1.0f;
  return model;
}

}  // namespace math
