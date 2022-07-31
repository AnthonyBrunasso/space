class Input {
public:
  static Input& Get();

  bool IsKeyDown(u32 keycode) const;
  bool IsKeyUp(u32 keycode) const;

  void SetKeyDown(u32 keycode, bool is_down);

  std::unordered_map<u32, bool> key_map_;
};

// Entity declarations as most the game will need these.
class Entity {
public:
  virtual ~Entity() = default;
  virtual void OnUpdate() {};
  u32 id_;
  v2f pos_;
  v2f dims_;
  proto::Entity2d::Type type_;
  // If true will dispatch virtual Update calls.
  bool has_update_ = false;
};

Entity* EntityGet(u32 entity_id);
void EntityDestroy(u32 entity_id);
void EntityUpdate();

Input& Input::Get() {
  static Input kInput;
  return kInput;
}

bool Input::IsKeyDown(u32 keycode) const {
  const bool* key_down = FindOrNull(key_map_, keycode);
  if (!key_down) return false;
  return *key_down;
}

bool Input::IsKeyUp(u32 keycode) const {
  return !IsKeyDown(keycode);
}

void Input::SetKeyDown(u32 keycode, bool is_down) {
  key_map_[keycode] = is_down;
}

#include "scheduler.cc"
#include "physics.cc"
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

  bool render_aabb_;
  bool render_physics_;

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
      Input::Get().SetKeyDown(event.key, true);
    } break;
    case KEY_UP: {
      //LOG(INFO, "Set key up %u", event.key);
      Input::Get().SetKeyDown(event.key, false);
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
  if (render_aabb_) {
    map_.RenderCollisionGeometry(scale_);
  }

  for (Character* character : kCharacters) {
    const AnimFrame2d* aframe = character->anim_.CurrentFrame();
    // Good to see if a character is missing animations for some reason
    if (!aframe) {
      rgg::RenderRectangle(Scale(Rectf(character->pos_, v2f(10.f, 10.f))), rgg::kRed);
      continue;
    }
    const rgg::Texture* texture_render_from = rgg::GetTexture(aframe->texture_id_);
    Rectf src = aframe->src_rect();
    Rectf dest = Scale(Rectf(character->pos_, v2f(src.width, src.height)));
    rgg::RenderTexture(*texture_render_from, src, dest);
    if (render_aabb_) {
      rgg::RenderLineRectangle(dest, rgg::kRed);
    }
  }

  if (render_physics_) {
    PDebugRender(scale_);
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
  kPNodes.clear();
  map_.Clear();  // Just in case the map is already hanging onto a texture.
  Map2d::LoadFromProtoFile(filename.c_str(), &map_);
  for (const proto::Entity2d& entity : map_.entities()) {
    EntityCreateFromProto(entity);
  } 
  for (const Rectf& rect : map_.collision_rects_) {
    PAddGeom(rect.Min(), rect.Dims());
  }
}

void Game::ChangeScale(r32 delta) {
  if (scale_ + delta > 0.f && scale_ + delta <= 15.f)
    scale_ += delta;
}

void Game::Main() {
  GameInitialize();

  EntityUpdate();
  // Physics updates happens after entity update to correct any intersecting collision
  // things before rendering. Otherwise we'd often see a frame of intersection.
  PUpdate();

  Render();
}
