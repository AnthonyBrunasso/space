#pragma once

#if 1

enum CameraMode {
  kCameraOverhead,
  kCameraBrowser,
  kCameraFirstPerson,
  kCameraFollow,
  kCameraMaxMode,
};

struct Camera {
  v3f position;
  v3f dir;
  v3f up = v3f(0.f, 1.f, 0.f);
  v2f viewport;
  CameraMode mode;
  v3f speed;
  v3f lerp_to;
  r32 lerpv = 1.f;
  bool camera_control = true;
};

struct CameraState {
  u32 camera_index;
  u32 camera_tag;
};

static CameraState kCameraState;

constexpr u32 kMaxCameras = 16;
constexpr u32 kMaxTags = 3;
constexpr u32 kLocalCameraTag = 2;

DECLARE_2D_ARRAY(Camera, kMaxTags, kMaxCameras);

void CameraResetAll() {
  for (s32 i = 0; i < kMaxTags; ++i) {
    kUsedCamera[i] = 0;
  }
}

Camera* CameraGet(u32 camera_tag, u32 camera_index) {
  return &kCamera[camera_tag][camera_index];
}

Camera* CameraGet(u32 camera_index) {
  return &kCamera[kLocalCameraTag][camera_index];
}

Camera* CameraGetCurrent() {
  return CameraGet(kCameraState.camera_tag, kCameraState.camera_index);
}

// Current camera will set its direction to be towards target
void CameraLookAt(const v3f& target) {
  Camera* c = CameraGetCurrent();
  if (!c) return;
  c->dir = math::Normalize(target - c->position);
}

// Will lerp along the XY axis to a certain ositino.
void CameraLerpToXY(const v2f& position) {
  Camera* c = CameraGetCurrent();
  if (!c) return;
  c->lerp_to.x = position.x;
  c->lerp_to.y = position.y;
  c->lerp_to.z = c->position.z;
  c->lerpv = c->lerpv >= 1.f ? 0.f : c->lerpv;
}

void CameraSetPositionXY(const v2f& position) {
  Camera* c = CameraGetCurrent();
  if (!c) return;
  c->position.x = position.x;
  c->position.y = position.y;
}

void CameraLerpToPositionXY(const v2f& position, r32 t) {
  Camera* c = CameraGetCurrent();
  if (!c) return;
  v2f pos = c->position.xy();
  v2f lpos = math::Lerp(pos, position, t);
  c->position.x = lpos.x;
  c->position.y = lpos.y;
}

v3f CameraLerpPosition() {
  Camera* c = CameraGetCurrent();
  if (!c) return v3f(0.f, 0.f, 0.f);
  return c->lerp_to;
}

v3f CameraPosition() {
  Camera* c = CameraGetCurrent();
  if (!c) return v3f(0.f, 0.f, 0.f);
  return c->position;
}

v3f CameraDirection() {
  Camera* c = CameraGetCurrent();
  if (!c) return v3f(0.f, 0.f, 0.f);
  return c->dir;
}

void CameraMove(const v3f& delta) {
  Camera* c = CameraGetCurrent();
  if (!c) return;
  c->position += delta;
}

void CameraSwitch(u32 camera_tag, u32 camera_index) {
  assert(camera_tag < kMaxTags);
  // Call CameraInit on new cameras.
  assert(camera_index < kUsedCamera[camera_tag]);
  kCameraState.camera_index = camera_index;
  kCameraState.camera_tag = camera_tag;
}

void CameraSwitch(u32 camera_index) {
  CameraSwitch(kLocalCameraTag, camera_index);
}

s32 CameraInit(u32 camera_tag, const Camera& camera) {
  Camera* c = UseCamera(camera_tag);
  if (!c) return 0;
  CameraSwitch(camera_tag, (u32)(kUsedCamera[camera_tag] - 1));
  *c = camera;
  return (s32)(kUsedCamera[camera_tag] - 1);
}

s32 CameraInit(Camera camera) {
  return CameraInit(kLocalCameraTag, camera);
}

b8 CameraHasLocalCamera() {
  return kUsedCamera[kLocalCameraTag] > 0;
}

void CameraOverhead(const PlatformEvent& event) {
  Camera* c = CameraGetCurrent();
  if (!c) return;
  v3f forward = math::Normalize(c->dir.xy());
  v3f right = math::Normalize(math::Cross(forward, c->up));
  if (!c->camera_control) return;
  switch (event.type) {
    case KEY_DOWN: {
      switch (event.key) {
        case 'w': {
          c->position += forward * c->speed.y;
        } break;
        case 'a': {
          c->position += -right * c->speed.x;
        } break;
        case 's': {
          c->position += -forward * c->speed.y;
        } break;
        case 'd': {
          c->position += right * c->speed.x;
        } break;
      }
    } break;
    case MOUSE_WHEEL: {
      c->position += (c->dir * event.wheel_delta * c->speed.z);
    } break;
    default: break;
  }
}

void CameraBrowser(const PlatformEvent& event) {
  Camera* c = CameraGetCurrent();
  if (!c) return;
  if (!c->camera_control) return;
  switch (event.type) {
    case KEY_DOWN: {
      switch (event.key) {
        case 'w': {
          c->position.y += c->speed.y;
        } break;
        case 'a': {
          c->position.x -= c->speed.x;
        } break;
        case 's': {
          c->position.y -= c->speed.y;
        } break;
        case 'd': {
          c->position.x += c->speed.x;
        } break;
      }
    } break;
    case MOUSE_WHEEL: {
      c->position += (c->dir * event.wheel_delta * c->speed.z);
    } break;
    default: break;
  }
}

void CameraFirstPerson(const PlatformEvent& event) {
  Camera* c = CameraGetCurrent();
  if (!c) return;
}

void CameraUpdate(u32 tag) {
  kCameraState.camera_tag = tag;
  Camera* c = CameraGetCurrent();
  // If there is some target to move to lerp to it.
  if (c->lerpv < 1.f) {
    c->position = math::Lerp(c->position, c->lerp_to, c->lerpv);
    c->lerpv += .001f;
  }
}

void CameraUpdate() {
  CameraUpdate(kLocalCameraTag);
}

void CameraUpdateEvent(const PlatformEvent& event, u32 tag) {
  kCameraState.camera_tag = tag;
  Camera* c = CameraGetCurrent();
  // Otherwise let the player control the camera.
  if (!c) return;
  switch (c->mode) {
    case kCameraFollow: {  // Relies on camera move.
    } break;
    case kCameraOverhead: {
      CameraOverhead(event);
    } break;
    case kCameraBrowser: {
      CameraBrowser(event);
    } break;
    case kCameraFirstPerson: {
      CameraFirstPerson(event);
    } break;
    default: break;
  }
}

void CameraUpdateEvent(const PlatformEvent& event) {
  CameraUpdateEvent(event, kLocalCameraTag);
}

Mat4f CameraLookAt() {
  Camera* c = CameraGetCurrent();
  if (!c) return math::Identity();
  return math::LookAt(c->position, c->position + c->dir * 1.f, c->up);
}

Mat4f CameraView() {
  Camera* c = CameraGetCurrent();
  if (!c) return math::Identity();
  switch (c->mode) {
    case kCameraFollow:
    case kCameraBrowser:
    case kCameraOverhead: {
      return CameraLookAt();
    } break;
    case kCameraFirstPerson: {
    } break;
    default: return math::Identity();
  };
  return math::Identity();
}

v3f CameraRayFromMouse(const v2f& screen) {
  Camera* c = CameraGetCurrent();
  if (!c) return {};
  v4f ray_clip(
      math::ScaleRange(screen.x, 0.f, c->viewport.x, -1.f, 1.f),
      math::ScaleRange(screen.y, 0.f, c->viewport.y, -1.f, 1.f),
      -1.f, 1.f);
  v4f ray_eye = math::Inverse(kObserver.projection) * ray_clip;
  ray_eye = v4f(ray_eye.x, ray_eye.y, -1.f, 0.f);
  Mat4f camera_inv = math::Inverse(CameraLookAt());
  return math::Normalize((camera_inv * ray_eye).xyz());
}

v3f CameraRayFromMouseToWorld(const v2f& screen, r32 plane_z) {
  v3f ray = CameraRayFromMouse(screen);
  v3f n(0.f, 0.f, 1.f);
  r32 t = -(math::Dot(CameraPosition(), n) + plane_z) / math::Dot(ray, n);
  v3f out = CameraPosition() + ray * t;
  return out;
}

#else

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

}

#endif
