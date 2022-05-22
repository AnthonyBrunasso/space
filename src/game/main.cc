#include "utils.cc"
#include "scheduler.cc"
#include "entity.cc"

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
    case KEY_DOWN: break;
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
