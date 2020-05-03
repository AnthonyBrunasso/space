#include "platform/platform.cc"
#include "audio/audio.cc"
#include "common/common.cc"
#include "gl/gl.cc"
#include "gfx/imui.cc"
#include "renderer/renderer.cc"
#include "renderer/texture.cc"
#include "renderer/mesh.cc"
#include "renderer/camera.cc"
#include "simulation/camera.cc"
#include "math/math.cc"

struct Mesh {
  rgg::Mesh mesh;
};

struct Sound {
  audio::Sound sound;
};

struct Texture {
  rgg::Texture texture;
};

DECLARE_HASH_MAP_STR(Mesh, 32);
DECLARE_HASH_MAP_STR(Sound, 32);
DECLARE_HASH_MAP_STR(Texture, 32);

rgg::Mesh* kCurrentMesh = nullptr;
rgg::Texture* kCurrentTexture = nullptr;

void
FileOBJCallback(const char* filename)
{
  if (strcmp(filesystem::GetFilenameExtension(filename), "obj") != 0) return;
  uint32_t len = strlen(filename);
  imui::TextOptions o;
  o.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
  if (imui::Text(filename, o).clicked) {
    Mesh* mesh = FindOrUseMesh(filename, len);
    if (!mesh->mesh.IsValid()) {
      if (!rgg::LoadOBJ(filename, &mesh->mesh)) {
        printf("Invalid mesh %s\n", filename);
      }
    }
    if (mesh->mesh.IsValid()) {
      kCurrentMesh = &mesh->mesh;
      kCurrentTexture = nullptr;
    }
  }
}

void
FileWAVCallback(const char* filename)
{
  if (strcmp(filesystem::GetFilenameExtension(filename), "wav") != 0) return;
  uint32_t len = strlen(filename);
  imui::TextOptions o;
  o.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
  if (imui::Text(filename, o).clicked) {
    Sound* sound = FindOrUseSound(filename, len);
    if (!sound->sound.IsValid()) {
      if (!audio::LoadWAV(filename, &sound->sound)) {
        printf("Invalid sound %s\n", filename);
      }
    }
    if (sound->sound.IsValid()) {
      audio::PlaySound(sound->sound, audio::Source());
    }
  }
}

void
FileTGACallback(const char* filename)
{
  if (strcmp(filesystem::GetFilenameExtension(filename), "tga") != 0) return;
  uint32_t len = strlen(filename);
  imui::TextOptions o;
  o.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
  if (imui::Text(filename, o).clicked) {
    Texture* texture = FindOrUseTexture(filename, len);
    if (!texture->texture.IsValid()) {
      if (!rgg::LoadTGA(filename, &texture->texture)) {
        printf("Invalid texture %s\n", filename);
      }
    }
    if (texture->texture.IsValid()) {
      kCurrentTexture = &texture->texture;
      kCurrentMesh = nullptr;
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
  imui::Text("Meshes");
  imui::Indent(2);
  filesystem::WalkDirectory("asset/", FileOBJCallback);
  imui::Indent(-2);
  imui::Text("Sounds");
  imui::Indent(2);
  filesystem::WalkDirectory("asset/", FileWAVCallback);
  imui::Indent(-2);
  imui::Text("Textures");
  imui::Indent(2);
  filesystem::WalkDirectory("asset/", FileTGACallback);
  imui::Indent(-2);
  imui::End();
}

void
RenderAxis()
{
  rgg::RenderLine(v3f(-1000.f, 0.f, 0.f), v3f(1000.f, 0.f, 0.f), v4f(1.f, 0.f, 0.f, 1.f));
  rgg::RenderLine(v3f(0.f, -1000.f, 0.f), v3f(0.f, 1000.f, 0.f), v4f(0.f, 1.f, 0.f, 1.f));
  rgg::RenderLine(v3f(0.f, 0.f, -1000.f), v3f(0.f, 0.f, 1000.f), v4f(0.f, 0.f, 1.f, 1.f));
}

int
main(int argc, char** argv)
{
  window::CreateInfo create_info;
  create_info.window_width = 1920;
  create_info.window_height = 1080;
  int window_result = window::Create("Space", create_info);
  if (!rgg::Initialize()) return 1;
  audio::Initialize();

  v2f size = window::GetWindowSize();
  auto* o = rgg::GetObserver();
  o->projection = math::Perspective(67.f, size.x / size.y, .1f, 1000.f);

  rgg::Camera camera;
  camera.position = v3f(0.f, 0.f, 10.f);
  camera.dir = v3f(0.f, 0.f, -1.f);
  camera.up = v3f(0.f, 1.f, 0.f);
  camera.mode = rgg::kCameraBrowser;
  camera.speed = .1f;
  rgg::CameraInit(camera);
  
  bool mouse_down = false;

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
          mouse_down = true;
          if (event.button == BUTTON_LEFT) {
            imui::MouseDown(event.position, event.button, 0);
          }
        } break;
        case MOUSE_UP: {
          mouse_down = false;
          imui::MouseUp(event.position, event.button, 0);
        } break;
        default: break;
      }
    }

    rgg::kObserver.position = rgg::CameraPosition();
    rgg::CameraUpdate();
    //rgg::CameraLookAt(v3f(0.f, 0.f, 0.f));
    o->view = rgg::CameraView();

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderAxis();
    if (kCurrentMesh && kCurrentMesh->IsValid()) {
      static float r = 0.f;
      if (mouse_down) {
        r += imui::MouseDelta(0).x;
      }
      rgg::RenderMesh(*kCurrentMesh, v3f(0.f, 0.f, 0.f), v3f(1.f, 1.f, 1.f),
                      Quatf(r, v3f(0.f, 1.f, 0.f)));
    } else if (kCurrentTexture && kCurrentTexture->IsValid()) {
      rgg::RenderTexture(
          *kCurrentTexture,
          Rectf(0, 0, kCurrentTexture->width, kCurrentTexture->height),
          Rectf(0, 0, kCurrentTexture->width / 100.f, kCurrentTexture->height / 100.f));
    }

    UI();

    imui::Render(0);
    imui::ResetAll();

    window::SwapBuffers();
    platform::sleep_usec(10*1000);
  }

  return 0;
}

