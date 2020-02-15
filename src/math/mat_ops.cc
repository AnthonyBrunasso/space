#pragma once

#include "mat.h"
#include "quat.h"
#include "vec.h"

namespace math
{
#define ONE_DEG_IN_RAD (2.0 * PI) / 360.0  // 0.017444444

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
Rotation(const Quatf& quat)
{
  Mat4f rotation;
  rotation.data_[0] = 1.f - 2.f * quat.y * quat.y - 2.f * quat.z * quat.z;
  rotation.data_[1] = 2.f * quat.x * quat.y - 2.f * quat.w * quat.z;
  rotation.data_[2] = 2.f * quat.x * quat.z + 2 * quat.w * quat.y;
  rotation.data_[3] = 0.0f;
  rotation.data_[4] = 2.f * quat.x * quat.y + 2.f * quat.w * quat.z;
  rotation.data_[5] = 1.f - 2.f * quat.x * quat.x - 2.f * quat.z * quat.z;
  rotation.data_[6] = 2.f * quat.y * quat.z - 2.f * quat.w * quat.x;
  rotation.data_[7] = 0.0f;
  rotation.data_[8] = 2.f * quat.x * quat.z - 2.f * quat.w * quat.y;
  rotation.data_[9] = 2.f * quat.y * quat.z + 2.f * quat.w * quat.x;
  rotation.data_[10] = 1.f - 2.f * quat.x * quat.x - 2.f * quat.y * quat.y;
  rotation.data_[11] = 0.0f;
  rotation.data_[12] = 0.0f;
  rotation.data_[13] = 0.0f;
  rotation.data_[14] = 0.0f;
  rotation.data_[15] = 1.0f;
  return rotation;
}

Mat4f
View(const v3f& translation, const Quatf& quat)
{
  auto mat = Rotation(quat).Transpose();
  Mat4f view;
  view.data_[0] = mat.data_[0];
  view.data_[1] = mat.data_[1];
  view.data_[2] = mat.data_[2];
  view.data_[3] = 0.0f;
  view.data_[4] = mat.data_[4];
  view.data_[5] = mat.data_[5];
  view.data_[6] = mat.data_[6];
  view.data_[7] = 0.0f;
  view.data_[8] = mat.data_[8];
  view.data_[9] = mat.data_[9];
  view.data_[10] = mat.data_[10];
  view.data_[11] = 0.0f;
  view.data_[12] = -translation.x;
  view.data_[13] = -translation.y;
  view.data_[14] = -translation.z;
  view.data_[15] = 1.0f;
  return view;
}

Mat4f
Perspective(float width, float height, float near_clip, float far_clip,
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
  return Mat4f(Sx,   0.0f, 0.0f,  0.0f,
               0.0f,   Sy, 0.0f,  0.0f,
               0.0f, 0.0f,   Sz, -1.0f,
               0.0f, 0.0f,   Pz,  0.0f);
}

Mat4f
Ortho(float right, float left, float top, float bottom, float far_clip,
      float near_clip)
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
Mat4f
Ortho2(float right, float left, float top, float bottom, float far_clip,
       float near_clip)
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

Mat4f
Model(const v3f& position, const v3f& scale,
      const math::Quatf& quat) {
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

}  // namespace math
