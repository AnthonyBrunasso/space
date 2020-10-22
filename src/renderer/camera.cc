#pragma once

#include "common/common.cc"
#include "renderer.cc"

namespace rgg
{

struct Camera {
 public:
  Camera() = delete;

  Camera(const v3f& position, const v3f& up, r32 yaw = -90.f, r32 pitch = 0.f)
    : position_(position), up_(up), yaw_(yaw), pitch_(pitch)
  {
    Update();
  }

  void
  PitchYawDelta(r32 pitch_delta, r32 yaw_delta)
  {
    yaw_ += yaw_delta;
    pitch_ += pitch_delta;

    if (pitch_ > 89.9f) pitch_ = 89.9f;
    if (yaw_ > 89.9f) yaw_ = 89.9f;

    Update();
  }

  void
  PitchYawDelta(v2f pitch_yaw)
  {
    PitchYawDelta(pitch_yaw.x, pitch_yaw.y);
  }

  void
  Zoom(r32 zoom_delta)
  {
    position_ += forward_ * zoom_delta;
  }

  void
  Translate(const v3f& delta)
  {
    position_ += right_ * delta.x;
    position_ += up_ * delta.y;
    position_ += forward_ * delta.z;
  }

  void
  Translate(const v3f& delta, const v3f& right,
            const v3f& up, const v3f& forward)
  {
    position_ += right * delta.x;
    position_ += up * delta.y;
    position_ += forward * delta.z;
  }

  const v3f&
  forward() const
  {
    return forward_;
  }

  const v3f&
  right() const
  {
    return right_;
  }

  const v3f&
  up() const
  {
    return up_;
  }

  const v3f&
  position() const
  {
    return position_;
  }

  Mat4f
  View()
  {
    return math::LookAt(position_, position_ + forward_, up_);
  }

  v3f
  RayFromScreen(const v2f& point, const v2f& viewport)
  {
    v4f ray_clip(
        math::ScaleRange(point.x, 0.f, viewport.x, -1.f, 1.f),
        math::ScaleRange(point.y, 0.f, viewport.y, -1.f, 1.f),
        -1.f, 1.f);
    v4f ray_eye = math::Inverse(kObserver.projection) * ray_clip;
    ray_eye = v4f(ray_eye.x, ray_eye.y, -1.f, 0.f);
    Mat4f camera_inv = math::Inverse(View());
    return math::Normalize((camera_inv * ray_eye).xyz());
  }

  v3f
  RayFromScreenToWorld(const v2f& point, const v2f& viewport, r32 plane_z)
  {
    v3f ray = RayFromScreen(point, viewport);
    v3f n(0.f, 0.f, 1.f);
    r32 t = -(math::Dot(position_, n) + plane_z) / math::Dot(ray, n);
    return position_ + ray * t;
  }

  void
  DebugRender()
  {
    v3f in_front = position_ + forward_ * 10.f;
    rgg::RenderLineCube(Cubef(in_front, v3f(1.f, 1.f, 1.f)), rgg::kRed);
    rgg::RenderLine(in_front, in_front + right(), rgg::kRed);
    rgg::RenderLine(in_front, in_front + up(), rgg::kGreen);
    rgg::RenderLine(in_front, in_front + forward(), rgg::kBlue);
  }

 private:
  void
  Update()
  {
    r32 yaw_rad = yaw_ * PI / 180.f;
    r32 pitch_rad = pitch_ * PI / 180.f;
    r32 cos_yaw = cos(yaw_rad);
    r32 sin_yaw = sin(yaw_rad);
    r32 cos_pitch = cos(pitch_rad);
    r32 sin_pitch = sin(pitch_rad);
    forward_ = math::Normalize(
        v3f(cos_yaw * cos_pitch, sin_pitch, sin_yaw * cos_pitch));
    right_ = math::Normalize(math::Cross(forward_, world_up_));
    up_ = math::Normalize(math::Cross(right_, forward_));
  }

  v3f position_;
  v3f forward_;
  v3f up_;
  v3f right_;
  v3f world_up_ = v3f(0.f, 1.f, 0.f);

  r32 yaw_;
  r32 pitch_;
};

}  // namespace rgg
