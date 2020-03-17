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
  v3f ray((2.f * screen.x) / dims.x - 1.f, (2.f * screen.y) / dims.y - 1.f,
          1.f);
  v4f ray_clip(ray.x, ray.y, -1.f, 1.f);
  //math::Print4x4Matrix(rgg::GetObserver()->projection);
  v4f ray_eye = math::Inverse(rgg::GetObserver()->projection) * ray_clip;
  //math::Print4x4Matrix(math::Inverse(rgg::GetObserver()->projection));
  //printf("%.3f,%.3f,%.3f\n", ray_eye.x, ray_eye.y, ray_eye.z);
  ray_eye = v4f(ray_eye.x, ray_eye.y, -1.f, 0.f);
  v3f ray_world = math::Normalize(
      (math::Inverse(rgg::GetObserver()->view) * ray_eye).xyz());
  float d = math::Length(cam->target - cam->position);
  v3f n(0.f, 0.f, 1.f);
  float t = -(math::Dot(cam->position, n) + d) / math::Dot(ray_world, n);
  v3f res = cam->position + ray_world * d;
  //res.z = 0.f;
  //printf("d:%.3f,%.3f,%.3f r:%.3f, %.3f, %.3f d:%.3f t:%.3f\n",
  //       ray_world.x, ray_world.y, ray_world.z, res.x, res.y, res.z, d, t);
  return res;
}

void
InitialCamera(Camera* cam)
{
  cam->position = v3f(0.f, 0.f, 100.f);
  cam->target = v3f(0.f, 0.f, 0.f);
}

}  // namespace camera
