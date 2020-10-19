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
  TranslateAlongX(r32 delta)
  {
    position_ += right_ * delta;
  }

  void
  TranslateAlongY(r32 delta)
  {
    position_ += up_ * delta;
  }

  void
  TranslateAlongZ(r32 delta)
  {
    position_ += forward_ * delta;
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
