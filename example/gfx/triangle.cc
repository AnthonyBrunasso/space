#include "math/math.cc"
#include "renderer/renderer.cc"
#include "renderer/camera.cc"
#include "renderer/imui.cc"

s32
main(s32 argc, char** argv)
{
  window::CreateInfo create_info;
  create_info.window_width = 1920;
  create_info.window_height = 1080;
  if (!window::Create("Simple Triangle", create_info)) {
    return 1;
  }

  if (!rgg::Initialize()) {
    return 1;
  }

  v2f dims = window::GetWindowSize();
  rgg::GetObserver()->projection =
      math::Perspective(67.f, dims.x / dims.y, .1f, 100.f);

  rgg::Camera camera;
  camera.position = v3f(0.f, 0.f, 40.f);
  camera.dir = v3f(0.f, 0.f, -1.f);
  camera.up = v3f(0.f, 1.f, 0.f);
  camera.mode = rgg::kCameraBrowser;
  camera.viewport = dims;
  camera.speed = v3f(1.f, 1.f, .1f);
  rgg::CameraInit(camera);

  platform::Clock clock;
  while (1) {
    platform::ClockStart(&clock);
    PlatformEvent event;
    while (window::PollEvent(&event)) {}

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(.1f, .1f, .13f, 1.f);

    rgg::RenderTriangle(v2f(0.f, 0.f), 100.f, v4f(1.f, 0.0f, 0.0f, .5f));

    window::SwapBuffers();
  }

  return 0;
}
