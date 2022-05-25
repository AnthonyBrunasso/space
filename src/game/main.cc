#include "utils.cc"

class Controller {
public:
  static Controller& Get();

  bool IsKeyDown(u32 keycode) const;
  bool IsKeyUp(u32 keycode) const;

  void SetKeyDown(u32 keycode, bool is_down);

  std::unordered_map<u32, bool> key_map_;
};

Controller& Controller::Get() {
  static Controller kController;
  return kController;
}

bool Controller::IsKeyDown(u32 keycode) const {
  const bool* key_down = FindOrNull(key_map_, keycode);
  if (!key_down) return false;
  return *key_down;
}

bool Controller::IsKeyUp(u32 keycode) const {
  return !IsKeyDown(keycode);
}

void Controller::SetKeyDown(u32 keycode, bool is_down) {
  key_map_[keycode] = is_down;
}

#include "scheduler.cc"
#include "entity.cc"
#include "render.cc"

class Game : public EditorRenderTarget {
public:
  void OnInitialize() override;
  void OnRender() override;
  void OnImGui() override;
  void OnFileSelected(const std::string& filename) override;
  void LoadMap(const std::string& filename);
  void ChangeScale(r32 delta);
  void Main();

private:
  Map2d map_;
};

static Game kGame;

void GameProcessEvent(const PlatformEvent& event) {
  kGame.UpdateCursor();
  switch(event.type) {
    case MOUSE_DOWN: break;
    case KEY_DOWN: {
      //LOG(INFO, "Set key down %u", event.key);
      Controller::Get().SetKeyDown(event.key, true);
    } break;
    case KEY_UP: {
      //LOG(INFO, "Set key up %u", event.key);
      Controller::Get().SetKeyDown(event.key, false);
    } break;
    case MOUSE_WHEEL: {
      if (kGame.IsMouseInsideEditorSurface() && !kGame.IsMouseInside()) {
        if (event.wheel_delta > 0) {
          kGame.ChangeScale(1.f);
        } else if (event.wheel_delta < 0) {
          kGame.ChangeScale(-1.f);
        }
      }
    } break;
    case MOUSE_UP: break;
    case NOT_IMPLEMENTED: break;
    default: break;
  }
}

void GameInitialize() {
  static bool do_once = true;
  if (!do_once) {
    return;
  }
  kGame.Initialize((s32)kEditor.render_viewport.width, (s32)kEditor.render_viewport.height);
  kGame.scale_ = 4.f;
  do_once = false;
}

void Game::OnInitialize() {
}

void Game::OnRender() {
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4 imcolor = style.Colors[ImGuiCol_WindowBg];
  // Fill the background with imgui's background color to maintain beauty.
  rgg::RenderRectangle(GetCameraRectScaled(), v4f(imcolor.x, imcolor.y, imcolor.z, imcolor.w));
  map_.Render(scale_);

  for (Character* character : Character::Array()) {
    rgg::RenderRectangle(Scale(Rectf(character->pos_, v2f(10.f, 10.f))), rgg::kRed);
  }
}

void Game::OnImGui() {
  ImGuiImage();
}

void Game::OnFileSelected(const std::string& filename) {
  if (filesystem::HasExtension(filename, "map")) {
    LoadMap(filename);
  }
}

void Game::LoadMap(const std::string& filename) {
  map_.Clear();  // Just in case the map is already hanging onto a texture.
  Map2d::LoadFromProtoFile(filename.c_str(), &map_);
  for (const proto::Entity2d& entity : map_.entities()) {
    EntityCreateFromProto(entity);
  } 
}

void Game::ChangeScale(r32 delta) {
  if (scale_ + delta > 0.f && scale_ + delta <= 15.f)
    scale_ += delta;
}

void Game::Main() {
  GameInitialize();

  EntityRunUpdates();

  Render();
}
