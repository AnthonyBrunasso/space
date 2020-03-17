#pragma once

// clang-format off
/*
 * While painting "broad strokes" of the game it it suitable to place camera into the simulation:
 *   ( this means player cameras are network synchronized )
 *   - platform inputs are dispatched unconditionally, and therefore easy to reason about
 *   - gameplay dev should be aware of the latency of inputs
 *   - increases information sharing, which is in the spirit of co-op
 *
 * The shipping game is unlikely to maintain this behavior. Update this list as we go:
 *   - Camera movement is a common activity, it should not feel "sluggish" due to network dependencies
 *   - How useful is it to see where other player's are looking? Does it add a liveliness to the world?
 *   - Window size changes the camera, so how do we handle initialization or a window resize during pause?
 * 
 */
// clang-format on
#include "math/math.cc"
#include "platform/platform.cc"
#include "renderer/renderer.cc"

struct Camera {
  v3f position;
  v3f target;
  v3f motion;
};

namespace camera
{
void
Update(Camera* cam)
{
  cam->target += cam->motion.xy();
  cam->position += cam->motion;
}

void
SetView(const Camera* cam, math::Mat4f* view)
{
  *view = math::LookAt(cam->position, cam->target, v3f(0.f, 1.f, 0.f));
}

// Transform matrix is orientating the click to take into consideration
// camera rotation / scale / translation.
v3f
ScreenToWorldSpace(const Camera* cam, const v3f screen)
{
  v2f dims = window::GetWindowSize();
  // Construct a ray taken into consideration camera's position in world space.
  v4f ray_clip(math::ScaleRange(screen.x, 0.f, dims.x, -1.f, 1.f), 
               math::ScaleRange(screen.y, 0.f, dims.y, -1.f, 1.f), 
               -1.f, 1.f);
  v4f ray_eye = math::Inverse(rgg::GetObserver()->projection) * ray_clip;
  ray_eye = v4f(ray_eye.x, ray_eye.y, -1.f, 0.f);
  v3f ray_world = math::Normalize(
      (math::Inverse(rgg::GetObserver()->view) * ray_eye).xyz());
  // Plane rooted at origin - 0.
  // I think you can use pixel depth info here if we need the ray to intersect
  // with taller objects.
  float d = 0;
  v3f n(0.f, 0.f, 1.f);
  float t = -(math::Dot(cam->position, n) + d) / math::Dot(ray_world, n);
  v3f res = cam->position + ray_world * t;
  return res;
}

void
InitialCamera(Camera* cam)
{
  cam->position = v3f(400.f, 400.f, 500.f);
  cam->target = v3f(400.f, 400.f, 0.f);
}

void
Move(Camera* cam, v3f pos)
{
  cam->position = v3f(pos.x, pos.y, cam->position.z);
  cam->target = v3f(pos.x, pos.y, 0.f);
}

}  // namespace camera
