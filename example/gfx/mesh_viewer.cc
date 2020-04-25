#include "platform/platform.cc"
#include "common/common.cc"
#include "gl/gl.cc"
#include "renderer/renderer.cc"
#include "renderer/mesh.cc"
#include "renderer/camera.cc"
#include "simulation/camera.cc"
#include "gfx/gfx.cc"
#include "math/math.cc"

struct Mesh {
  rgg::Mesh mesh;
};

DECLARE_HASH_MAP_STR(Mesh, 32);

rgg::Mesh* kCurrentMesh = nullptr;

// Probably a common function
const char* GetFilenameExtension(const char* filename)
{
  const char* dot = strrchr(filename, '.');
  if(!dot || dot == filename) return "";
  return dot + 1;
}

void
FileCallback(const char* filename)
{
  if (strcmp(GetFilenameExtension(filename), "obj") != 0) return;
  imui::TextOptions o;
  o.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
  if (imui::Text(filename, o).clicked) {
    uint32_t len = strlen(filename);
    Mesh* mesh = FindMesh(filename, len);
    if (!mesh) mesh = UseMesh(filename, len);
    if (!mesh->mesh.IsValid()) {
      if (!LoadOBJ(filename, &mesh->mesh)) {
        printf("Invalid mesh %s\n", filename);
      }
    }
    if (mesh->mesh.IsValid()) {
      kCurrentMesh = &mesh->mesh;
    }
  }
}

void
UI()
{
  v2f screen = window::GetWindowSize();
  static bool dir_enable = true;
  static v2f dir_pos(300.f, screen.y - 300.f);
  imui::PaneOptions options;
  options.max_width = 315.f;
  imui::Begin("Assets", 0, options, &dir_pos, &dir_enable);
  filesystem::WalkDirectory("asset/*", FileCallback);
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
  camera.position = v3f(0.f, 0.f, 10.f);
  camera.dir = v3f(0.f, 0.f, -1.f);
  camera.up = v3f(0.f, 1.f, 0.f);
  camera.mode = rgg::kCameraOverhead;
  camera.speed = 3.f;
  rgg::CameraInit(camera);

  //rgg::Mesh mesh;
  //rgg::LoadOBJ("asset/gear.obj", &mesh);
  
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
        case MOUSE_UP: {
          imui::MouseUp(event.position, event.button, 0);
        } break;
        default: break;
      }
    }

    rgg::kObserver.position = rgg::CameraPosition();
    rgg::CameraUpdate();
    rgg::CameraLookAt(v3f(0.f, 0.f, 0.f));
    o->view = rgg::CameraView();

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderAxis();
    if (kCurrentMesh && kCurrentMesh->IsValid()) {
      rgg::RenderMesh(*kCurrentMesh, v3f(0.f, 0.f, 0.f), v3f(1.f, 1.f, 1.f), Quatf(),
                      v4f(1.f, 1.f, 1.f, 1.f));
    }

    UI();

    imui::Render(0);
    imui::ResetAll();

    window::SwapBuffers();
    platform::sleep_usec(10*1000);
  }

  return 0;
}

