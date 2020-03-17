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
  o->projection = math::Perspective(size.x, size.y, 0.01f, 10000.f, 67.f);
  v3f cpos(0.f, 0.f, 100.f);
  v3f ctgt(0.f, 0.f, 0.f);
  v3f cup(0.f, 1.f, 0.f);

  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {
      switch (event.type) {
        case KEY_DOWN: {
          switch (event.key) {
            case 'w': {
              printf("??\n");
              cpos.y += 1.f;
              ctgt.y += 1.f;
            } break;
            case 'a': {
              cpos.x -= 1.f;
              ctgt.x -= 1.f;
            } break;
            case 's': {
              cpos.y -= 1.f;
              ctgt.y -= 1.f;
            } break;
            case 'd': {
              cpos.x += 1.f;
              ctgt.x += 1.f;
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

    o->view = math::LookAt(cpos, ctgt, cup);

    //math::Print4x4Matrix(o->view);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    rgg::RenderLineCube(math::Cubef(v3f(0.f, 0.f, 0.f), 30.f, 30.f, 30.f),
                        v4f(1.f, 1.f, 1.f, 1.f));
#if 1
    rgg::RenderLine(cpos + v3f(0.f, 0.f, -5.f), v3f(30.f, 30.f, 30.f),
                    v4f(1.f, 0.f, 0.f, 1.f));

    rgg::RenderLine(cpos + v3f(0.f, 0.f, -5.f), v3f(30.f, 0.f, 30.f),
                    v4f(0.f, 1.f, 0.f, 1.f));

    rgg::RenderLine(cpos + v3f(0.f, 0.f, -5.f), v3f(0.f, 30.f, 30.f),
                    v4f(0.f, 0.f, 1.f, 1.f));

    rgg::RenderLine(cpos + v3f(0.f, 0.f, -5.f), v3f(0.f, 0.f, 30.f),
                    v4f(0.f, 4.f, 4.f, 1.f));
#endif
    window::SwapBuffers();
    platform::sleep_usec(10*1000);
  }

  return 0;
}

