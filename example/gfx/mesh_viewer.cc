#include "platform/platform.cc"
#include "gl/gl.cc"
#include "renderer/renderer.cc"
#include "renderer/mesh.cc"
#include "renderer/camera.cc"
#include "simulation/camera.cc"
#include "gfx/gfx.cc"
#include "math/math.cc"

void
UI()
{
}

void
RenderAxis()
{
  rgg::RenderLine(v3f(0.f, 0.f, 0.f), v3f(1000.f, 0.f, 0.f), v4f(1.f, 0.f, 0.f, 1.f));
  rgg::RenderLine(v3f(0.f, 0.f, 0.f), v3f(0.f, 1000.f, 0.f), v4f(0.f, 1.f, 0.f, 1.f));
  rgg::RenderLine(v3f(0.f, 0.f, 0.f), v3f(0.f, 0.f, 1000.f), v4f(0.f, 0.f, 1.f, 1.f));
}

int
main(int argc, char** argv)
{
  window::CreateInfo create_info;
  create_info.window_width = 1920;
  create_info.window_height = 1080;
  gfx::Initialize(create_info);

  v2f size = window::GetWindowSize();
  auto* o = rgg::GetObserver();
  o->projection = math::Perspective(67.f, size.x / size.y, .1f, 1000.f);

  rgg::Camera camera;
  camera.position = v3f(0.f, 0.f, 10.f);
  camera.dir = v3f(0.f, 0.f, -1.f);
  camera.up = v3f(0.f, 1.f, 0.f);
  camera.mode = rgg::kCameraOverhead;
  camera.speed = 3.f;
  rgg::CameraInit(camera);

  rgg::Mesh mesh;
  rgg::LoadOBJ("asset/gear.obj", &mesh);
  
  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {
      rgg::CameraUpdateEvent(event);
      switch (event.type) {
        case KEY_DOWN: {
          switch (event.key) {
            case 'k': {
            } break;
            case 'h': {
            } break;
            case 'j': {
            } break;
            case 'l': {
            } break;
            case 27 /* ESC */: {
              exit(1);
            } break;
          }
        } break;
        case MOUSE_DOWN: {
          if (event.button == BUTTON_LEFT) {
            imui::MouseDown(event.position, event.button, 0);
          }
        } break;
        default: break;
      }
    }

    rgg::kObserver.position = rgg::CameraPosition();
    rgg::CameraUpdate();

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, 0);


    o->view = rgg::CameraView();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderAxis();
    rgg::CameraLookAt(v3f(0.f, 0.f, 0.f));
    rgg::RenderMesh(mesh, v3f(0.f, 0.f, 0.f), v3f(1.f, 1.f, 1.f), Quatf(),
                    v4f(1.f, 1.f, 1.f, 1.f));

    UI();

    imui::Render(0);
    imui::ResetAll();

    window::SwapBuffers();
    platform::sleep_usec(10*1000);
  }

  return 0;
}

