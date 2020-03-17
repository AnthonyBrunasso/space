#include "platform/platform.cc"
#include "gl/gl.cc"
#include "renderer/renderer.cc"
#include "simulation/camera.cc"
#include "gfx/gfx.cc"

int
main(int argc, char** argv)
{
  gfx::Initialize();

  v2f size = window::GetWindowSize();
  auto* o = rgg::GetObserver();
  o->projection =
    math::Perspective(size.x, size.y, 10.f, 0.f, 45.0f);
  v3f cpos(0.f, 0.f, 200.f);
  v3f aim(0.f, 0.78f, -1.f);
  math::Quatf cori(0.f, math::Normalize(aim));


  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {
      switch (event.type) {
        case KEY_DOWN: {
          switch (event.key) {
            case 'w': {
              printf("??\n");
              cpos.y += 1.f;
            } break;
            case 'a': {
              cpos.x -= 1.f;
            } break;
            case 's': {
              cpos.y -= 1.f;
            } break;
            case 'd': {
              cpos.x += 1.f;
            } break;
          }
        } break;
        case MOUSE_DOWN: {
          if (event.button == BUTTON_LEFT) {
            imui::MouseClick(event.position, event.button);
          }
        } break;
        case MOUSE_WHEEL: {
          printf("??\n");
          cpos.z += (-0.1f * event.wheel_delta);
        } break;
        default: break;
      }
    }

    o->view = math::View(cpos, cori);

    math::Print4x4Matrix(o->view);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    rgg::RenderRectangle(math::Rectf(30.f, 30.f, 30.f, 30.f), v4f(1.f, 1.f, 1.f, 1.f));

    rgg::RenderCube(math::Cubef(v3f(0.f, 0.f, 0.f), 30.f, 30.f, -30.f),
                    v4f(1.f, 1.f, 1.f, 1.f));

    window::SwapBuffers();
    platform::sleep_usec(10*1000);
  }

  return 0;
}

