#pragma once

#include "common/common.cc"
#include "renderer.cc"

namespace rgg
{

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

void
CameraResetAll()
{
  for (s32 i = 0; i < kMaxTags; ++i) {
    kUsedCamera[i] = 0;
  }
}

Camera*
CameraGetCurrent()
{
  return &kCamera[kCameraState.camera_tag][kCameraState.camera_index];
}

// Current camera will set its direction to be towards target
void
CameraLookAt(const v3f& target)
{
  Camera* c = CameraGetCurrent();
  if (!c) return;
  c->dir = math::Normalize(target - c->position);
}

// Will lerp along the XY axis to a certain ositino.
void
CameraLerpToXY(const v2f& position)
{
  Camera* c = CameraGetCurrent();
  if (!c) return;
  c->lerp_to.x = position.x;
  c->lerp_to.y = position.y;
  c->lerp_to.z = c->position.z;
  c->lerpv = c->lerpv >= 1.f ? 0.f : c->lerpv;
}

void
CameraSetPositionXY(const v2f& position)
{
  Camera* c = CameraGetCurrent();
  if (!c) return;
  c->position.x = position.x;
  c->position.y = position.y;
}

v3f
CameraPosition()
{
  Camera* c = CameraGetCurrent();
  if (!c) return v3f(0.f, 0.f, 0.f);
  return c->position;
}

v3f
CameraDirection()
{
  Camera* c = CameraGetCurrent();
  if (!c) return v3f(0.f, 0.f, 0.f);
  return c->dir;
}

void
CameraMove(const v3f& delta)
{
  Camera* c = CameraGetCurrent();
  if (!c) return;
  c->position += delta;
}

void
CameraSwitch(u32 camera_tag, u32 camera_index)
{
  assert(camera_tag < kMaxTags);
  // Call CameraInit on new cameras.
  assert(camera_index < kUsedCamera[camera_tag]);
  kCameraState.camera_index = camera_index;;
  kCameraState.camera_tag = camera_tag;
}

void
CameraInit(u32 camera_tag, const Camera& camera)
{
  Camera* c = UseCamera(camera_tag);
  if (!c) return;
  CameraSwitch(camera_tag, kUsedCamera[camera_tag] - 1);
  *c = camera;
}

void
CameraInit(Camera camera)
{
  CameraInit(kLocalCameraTag, camera);
}

void
CameraOverhead(const PlatformEvent& event)
{
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

void
CameraBrowser(const PlatformEvent& event)
{
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

void
CameraFirstPerson(const PlatformEvent& event)
{
  Camera* c = CameraGetCurrent();
  if (!c) return;
}

void
CameraUpdate(u32 tag)
{
  kCameraState.camera_tag = tag;
  Camera* c = CameraGetCurrent();
  // If there is some target to move to lerp to it.
  if (c->lerpv < 1.f) {
    c->position = math::Lerp(c->position, c->lerp_to, c->lerpv);
    c->lerpv += .001f;
  }
}

void
CameraUpdate()
{
  CameraUpdate(kLocalCameraTag);
}

void
CameraUpdateEvent(const PlatformEvent& event, u32 tag)
{
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

void
CameraUpdateEvent(const PlatformEvent& event)
{
  CameraUpdateEvent(event, kLocalCameraTag);
}

Mat4f
CameraLookAt()
{
  Camera* c = CameraGetCurrent();
  if (!c) return math::Identity();
  return math::LookAt(c->position, c->position + c->dir * 1.f, c->up);
}

Mat4f
CameraView()
{
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

v3f
CameraRayFromMouse(const v2f& screen)
{
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

v3f
CameraRayFromMouseToWorld(const v2f& screen, r32 plane_z)
{
  v3f ray = CameraRayFromMouse(screen);
  v3f n(0.f, 0.f, 1.f);
  r32 t = -(math::Dot(CameraPosition(), n) + plane_z) / math::Dot(ray, n);
  return CameraPosition() + ray * t;
}

}
