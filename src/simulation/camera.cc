
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
  math::Quatf orientation;
  // Vector of movement for the camera
  //   Useful for stateful translation (i.e. the duration of keypress)
  v2f motion;
};

namespace camera
{
void
Update(Camera* cam)
{
  cam->position += cam->motion;
}

void
SetView(const Camera* cam, math::Mat4f* view)
{
  *view = math::View(cam->position, cam->orientation);
}

// Transform matrix is orientating the click to take into consideration
// camera rotation / scale / translation.
v3f
ScreenToWorldSpace(const Camera* cam, const v3f screen)
{
  math::Mat4f camera_transform = math::Rotation(cam->orientation) *
                                 math::Translation(cam->position);
  v3f ret = camera_transform * screen;

  return ret;
}

void
InitialCamera(Camera* cam)
{
  cam->position = v3f(400.f, 400.f, 0.f);
  // cam->orientation.Set(45.f, v3f(0.f, 0.f, 1.f));
}

void
OrientCamera(const v3f& dir, Camera* cam)
{
  cam->orientation.Set(0.f, dir);
}

}  // namespace camera
