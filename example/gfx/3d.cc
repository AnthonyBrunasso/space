#include "platform/platform.cc"
#include "gl/gl.cc"
#include "renderer/renderer.cc"
#include "renderer/camera.cc"
#include "simulation/camera.cc"
#include "gfx/gfx.cc"
#include "math/math.cc"

v3f cpos(0.f, 0.f, 100.f);
v3f ctgt(0.f, 0.f, 0.f);
v3f cup(0.f, 1.f, 0.f);
float speed = 10.f;
v3f cubepos(0.f, 0.f, 0.f);
v3f cubedim(15.f, 15.f, 15.f);
v2f rp;

void
UI()
{
  char buffer[64];
  auto dims = window::GetWindowSize();
  imui::TextOptions text_options;
  text_options.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
  imui::PaneOptions pane_options;
  imui::Begin(v2f(20.f, dims.y - 50.f), 0, pane_options);
  snprintf(buffer, 64, "cpos: (%.3f,%.3f,%.3f)", cpos.x, cpos.y, cpos.z);
  imui::Text(buffer);
  snprintf(buffer, 64, "ctgt: (%.3f,%.3f,%.3f)", ctgt.x, ctgt.y, ctgt.z);
  imui::Text(buffer);
  if (imui::Text("Toggle Mode", text_options).clicked) {
    rgg::Camera* c = rgg::CameraGetCurrent();
    if (c) {
      c->mode = (rgg::CameraMode)((c->mode + 1) % rgg::kCameraMaxMode);
    }
  }
  if (imui::Text("Create Camera", text_options).clicked) {
    rgg::Camera camera;
    camera.position = cpos;
    camera.dir = math::Normalize(ctgt - cpos);
    camera.up = v3f(0.f, 1.f, 0.f);
    camera.mode = rgg::kCameraControl;
    rgg::CameraInit(camera);
  }
  imui::Text("Local Cameras"); 
  imui::Indent(2);
  for (int i = 0; i < rgg::kUsedCamera[rgg::kLocalCameraTag]; ++i) {
    rgg::Camera* c = &rgg::kCamera[rgg::kLocalCameraTag][i];
    snprintf(buffer, 64, "%i pos(%.2f,%.2f,%.2f) dir(%.2f,%.2f,%.2f) mode(%i)", i,
             c->position.x, c->position.y, c->position.y,
             c->dir.x, c->dir.y, c->dir.y, c->mode);
    if (imui::Text(buffer, text_options).clicked) {
      rgg::CameraSwitch(rgg::kLocalCameraTag, i);
    }
  }
  imui::Indent(-2);
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

  rgg::Camera camera;
  camera.position = cpos;
  camera.dir = math::Normalize(ctgt - cpos);
  camera.up = v3f(0.f, 1.f, 0.f);
  camera.mode = rgg::kCameraControl;
  rgg::CameraInit(camera);
  
  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {
      rgg::CameraUpdate(event);
      switch (event.type) {
        case KEY_DOWN: {
          switch (event.key) {
            case 'k': {
              ctgt.y += speed;
            } break;
            case 'h': {
              ctgt.x -= speed;
            } break;
            case 'j': {
              ctgt.y -= speed;
            } break;
            case 'l': {
              ctgt.x += speed;
            } break;
            case 27 /* ESC */: {
              exit(1);
            } break;
          }
          rp = math::RandomPointOnRect(math::Rectf(
              cubepos.x, cubepos.y, cubedim.x, cubedim.y));
        } break;
        case MOUSE_DOWN: {
          if (event.button == BUTTON_LEFT) {
            imui::MouseClick(event.position, event.button, 0);
          }
        } break;
        default: break;
      }
    }

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, 0);

    rgg::CameraFollow(ctgt);
    o->view = rgg::CameraView();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    rgg::kObserver.position = rgg::CameraPosition();

    rgg::RenderLineCube(math::Cubef(ctgt, v3f(15.f, 15.f, 15.f)),
                        v4f(1.f, 0.f, 0.f, 1.f));

    RenderAxis();

    UI();

    imui::Render(0);
    imui::ResetAll();

    window::SwapBuffers();
    platform::sleep_usec(10*1000);
  }

  return 0;
}

