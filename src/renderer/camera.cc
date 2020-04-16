#pragma once

#include "../common/common.cc"

namespace rgg
{

enum CameraMode {
  kCameraOverhead,
  kCameraFirstPerson,
  kCameraMaxMode,
};

struct Camera {
  v3f position;
  v3f dir;
  v3f up = v3f(0.f, 1.f, 0.f);
  CameraMode mode;
  float speed = 10.f;
  v3f lerp_to;
  float lerpv = 1.f;
};

struct CameraState {
  uint32_t camera_index;
  uint32_t camera_tag;
};

static CameraState kCameraState;

constexpr uint32_t kMaxCameras = 16;
constexpr uint32_t kMaxTags = 3;
constexpr uint32_t kLocalCameraTag = 2;

DECLARE_2D_ARRAY(Camera, kMaxTags, kMaxCameras);

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

v3f
CameraPosition()
{
  Camera* c = CameraGetCurrent();
  if (!c) return v3f(0.f, 0.f, 0.f);
  return c->position;
}

void
CameraSwitch(uint32_t camera_tag, uint32_t camera_index)
{
  printf("%i %i\n", camera_tag, camera_index);
  assert(camera_tag < kMaxTags);
  // Call CameraInit on new cameras.
  assert(camera_index < kUsedCamera[camera_tag]);
  kCameraState.camera_index = camera_index;;
  kCameraState.camera_tag = camera_tag;
}

void
CameraInit(uint32_t camera_tag, const Camera& camera)
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
  switch (event.type) {
    case KEY_DOWN: {
      switch (event.key) {
        case 'w': {
          c->position.y += c->speed;
        } break;
        case 'a': {
          c->position.x -= c->speed;
        } break;
        case 's': {
          c->position.y -= c->speed;
        } break;
        case 'd': {
          c->position.x += c->speed;
        } break;
      }
    } break;
    case MOUSE_WHEEL: {
      c->position.z += (-10.f * event.wheel_delta);
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
CameraUpdate(uint32_t tag)
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
CameraUpdateEvent(const PlatformEvent& event, uint32_t tag)
{
  kCameraState.camera_tag = tag;
  Camera* c = CameraGetCurrent();
  // Otherwise let the player control the camera.
  if (!c) return;
  switch (c->mode) {
    case kCameraOverhead: {
      CameraOverhead(event);
    } break;
    case kCameraFirstPerson: {
      CameraFirstPerson(event);
    } break;
  }
}

void
CameraUpdateEvent(const PlatformEvent& event)
{
  CameraUpdateEvent(event, kLocalCameraTag);
}

Mat4f
CameraView()
{
  Camera* c = CameraGetCurrent();
  if (!c) return math::Identity();
  switch (c->mode) {
    case kCameraOverhead: {
      return math::LookAt(c->position, c->position + c->dir * 1.f, c->up);
    } break;
    case kCameraFirstPerson: {
    } break;
    default: return math::Identity();
  };
  return math::Identity();
}

}
