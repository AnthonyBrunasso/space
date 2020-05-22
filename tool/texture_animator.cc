#include "platform/platform.cc"
#include "audio/audio.cc"
#include "common/common.cc"
#include "gl/gl.cc"
#include "animation/sprite.cc"
#include "renderer/renderer.cc"
#include "renderer/texture.cc"
#include "renderer/mesh.cc"
#include "renderer/camera.cc"
#include "renderer/imui.cc"
#include "math/math.cc"

struct Texture {
  rgg::Texture texture;
};

struct Sprite {
  animation::Sprite sprite;
};

DECLARE_HASH_MAP_STR(Texture, 32);
DECLARE_HASH_MAP_STR(Sprite, 32);

rgg::Texture* kCurrentTexture = nullptr;
animation::Sprite* kCurrentSprite = nullptr;

static uint32_t kFrame;

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
      rgg::TextureInfo info;
      if (strcmp(filename, "asset/adventurer.tga") == 0) {
        info.min_filter = GL_NEAREST_MIPMAP_NEAREST;
        info.mag_filter = GL_NEAREST;
      }
      if (!rgg::LoadTGA(filename, info, &texture->texture)) {
        printf("Invalid texture %s\n", filename);
      }
    }
    if (texture->texture.IsValid()) {
      kCurrentTexture = &texture->texture;
    }
  }
}

void
FileAnimCallback(const char* filename)
{
  if (strcmp(filesystem::GetFilenameExtension(filename), "anim") != 0) return;
  uint32_t len = strlen(filename);
  imui::TextOptions o;
  o.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
  if (imui::Text(filename, o).clicked) {
    Sprite* sprite = FindOrUseSprite(filename, len);
    if (!sprite->sprite.IsValid()) {
      animation::LoadAnimation(filename, &sprite->sprite);
    }
    kCurrentSprite = &sprite->sprite;
  }
}

void
UI()
{
  v2f screen = window::GetWindowSize();
  {
    static bool dir_enable = true;
    static v2f dir_pos(1500.f, screen.y - 200.f);
    imui::PaneOptions options;
    options.max_width = 315.f;
    imui::Begin("Textures", 0, options, &dir_pos, &dir_enable);
    imui::Space(imui::kVertical, 3.f);
    imui::Text("Textures");
    imui::Indent(2);
    filesystem::WalkDirectory("asset/", FileTGACallback);
    imui::Indent(-2);
    imui::Text("Animations");
    imui::Indent(2);
    filesystem::WalkDirectory("asset/", FileAnimCallback);
    imui::Indent(-2);
    imui::End();
  }

  if (kCurrentSprite) {
    bool enable = true;
    static v2f pos(1500.f, screen.y - 600.f);
    imui::PaneOptions options;
    options.max_width = 315.f;
    imui::Begin("Animations", 0, options, &pos, &enable);
    for (int i = 0; i < kCurrentSprite->label_size; ++i) {
      animation::Label* l = &kCurrentSprite->label[i];
      imui::TextOptions o;
      o.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
      if (imui::Text(l->name, o).clicked) {
        printf("Set label %s\n", l->name);
        animation::SetLabel(l->name, kCurrentSprite);
      }
    }
    imui::End();
  }
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
  o->projection = math::Ortho(size.x, 0.f, size.y, 0.f, -100.f, 100.f);

  rgg::Camera camera;
  camera.position = v3f(0.f, 1.f, 1.f);
  camera.dir = v3f(0.f, 0.f, -1.f);
  camera.up = v3f(0.f, 1.f, 0.f);
  camera.mode = rgg::kCameraBrowser;
  camera.speed = v3f(5.f, 5.f, 0.1f);
  rgg::CameraInit(camera);
  
  bool mouse_down = false;

  while (!window::ShouldClose()) {
    ++kFrame;
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
    if (kCurrentTexture && kCurrentTexture->IsValid()) {
      rgg::RenderTexture(
          *kCurrentTexture,
          Rectf(0, 0, kCurrentTexture->width, kCurrentTexture->height),
          Rectf(-kCurrentTexture->width, 0,
                kCurrentTexture->width, kCurrentTexture->height));
      if (kCurrentSprite) {
        rgg::RenderTexture(
            *kCurrentTexture,
            animation::Update(kCurrentSprite),
            Rectf(0, 0, 50, 37));
      }
    }

    UI();

    imui::Render(0);
    imui::ResetAll();

    window::SwapBuffers();
    platform::SleepUsec(10*1000);
  }

  return 0;
}

