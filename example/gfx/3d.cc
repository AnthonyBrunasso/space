#include "platform/platform.cc"
#include "gl/gl.cc"
#include "renderer/renderer.cc"
#include "simulation/camera.cc"
#include "gfx/gfx.cc"

v3f cpos(0.f, 0.f, 100.f);
v3f ctgt(0.f, 0.f, 0.f);
v3f cup(0.f, 1.f, 0.f);
float speed = 10.f;
v3f cubepos(0.f, 0.f, 0.f);

void
UI()
{
  char buffer[64];
  auto dims = window::GetWindowSize();
  imui::PaneOptions pane_options;
  imui::Begin(v2f(20.f, dims.y - 50.f), 0, pane_options);
  snprintf(buffer, 64, "Camera: (%.3f,%.3f,%.3f)", cpos.x, cpos.y, cpos.z);
  imui::Text(buffer);
  snprintf(buffer, 64, "Cube: (%.3f,%.3f,%.3f)", cubepos.x, cubepos.y, cubepos.z);
  imui::Text(buffer);
  imui::End();
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
  
  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {
      switch (event.type) {
        case KEY_DOWN: {
          switch (event.key) {
            case 'w': {
              cpos.y += speed;
              ctgt.y += speed;
            } break;
            case 'a': {
              cpos.x -= speed;
              ctgt.x -= speed;
            } break;
            case 's': {
              cpos.y -= speed;
              ctgt.y -= speed;
            } break;
            case 'd': {
              cpos.x += speed;
              ctgt.x += speed;
            } break;
            case 'k': {
              cubepos.z -= speed;
            } break;
            case 'h': {
              cubepos.x -= speed;
            } break;
            case 'j': {
              cubepos.z += speed;
            } break;
            case 'l': {
              cubepos.x += speed;
            } break;
          }
        } break;
        case MOUSE_DOWN: {
          if (event.button == BUTTON_LEFT) {
            imui::MouseClick(event.position, event.button, 0);
          }
        } break;
        case MOUSE_WHEEL: {
          cpos.z += (-10.f * event.wheel_delta);
        } break;
        default: break;
      }
    }

    o->view = math::LookAt(cpos, ctgt, cup);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    rgg::kObserver.position = cpos;

    rgg::RenderRectangle(math::Rectf(cubepos.x, cubepos.y, 10.f, 10.f),
                         cubepos.z, v4f(1.f, 1.f, 1.f, 1.f));

    RenderAxis();

    UI();

    imui::Render(0);
    imui::Reset();

    window::SwapBuffers();
    platform::sleep_usec(10*1000);
  }

  return 0;
}

