#pragma once


// ImGui
//    * Top left is origin.

enum EditorMode {
  EDITOR_MODE_GAME = 0,
  EDITOR_MODE_SPRITE_ANIMATOR = 1,
  EDITOR_MODE_MAP_MAKER = 2,
  EDITOR_MODE_ENTITY_CREATOR = 3,
};

static const s32 kViewportTabCount = 4;

static const char* kViewportTabs[kViewportTabCount] = {
  "Game",
  "Animator",
  "Map Maker",
  "Entity Creator",
};

static bool kOpened[kViewportTabCount] = { true, true, true, true };

class EditorRenderTarget;

struct Editor {
  u64 frame_rate = 60;
  window::CreateInfo window_create_info;
  platform::Clock clock;
  Rectf render_viewport_in_editor;
  Rectf render_viewport;
  EditorRenderTarget* current = nullptr;
  EditorMode mode;
};

struct EditorGrid {
  s32 cell_width = 16;
  s32 cell_height = 16;
  v2f origin = v2f(0.f, 0.f);
  v2f GetOrigin() const {
    return origin + origin_offset;
  }
  v2f origin_offset = v2f(0.f, 0.f);
};


struct EditorCursor {
  // x-y coordinates of the cursor in the editor, assuming bottom left is origin.
  v2f global_screen;
  // x-y coordinates of the cursor in the game / asset viewer / etc viewport. This will be influenced by
  // camera / scale / etc. This is in the space that'd be used during rendering.
  v2f world_scaled;
  // x-y coordinates of the cursor in the game / asset viewer / etc viewport. This is the space that all game
  // logic should take place in before scaling for rendering.
  v2f world;
  // x-y coordinate of the nearest edge given by the grid.
  v2f world_clamped;
  // x-y coordinates of the cursor in the game / asset viewer, assuming bottom left is origin.
  v2f local_screen;
  // The grid cell of the current grid.
  Rectf world_grid_cell;
  // True when the cursor is in the game / asset viewer.
  bool is_in_viewport = false;
};

static Editor kEditor;

static s32 kExplorerStart = 0;
static s32 kExplorerWidth = 300;

static s32 kRenderViewStart = 300;

static s32 kFrameRendererHeight = 220;

// Turns paths like 'C:\projects\space\asset\file.png' to '.\asset\file.png'
// Turns paths like '/users/anthony/projects/space/asset/file.png' to './asset/file.png'
std::string GetAssetRelative(const char* full_path) {
#ifdef _WIN32
  std::string r = ".\\";
#else
  std::string r = "./";
#endif
  const char* sidx = strstr(full_path, "asset");
  if (!sidx) {
    sidx = strstr(full_path, "gamedata");
  }
  assert(sidx);
  r.append(sidx);
  return r;
}

std::string GetAssetRelative(const std::string& full_path) {
  return GetAssetRelative(full_path.c_str());
}

static const std::vector<std::string> kEditorKnownAssetExtensions = {
  "tga",
  "png",
  "anim",
  "map",
  "entity",
};

bool EditorCanLoadAsset(const std::string& name) {
  for (const std::string& ext : kEditorKnownAssetExtensions) {
    if (filesystem::HasExtension(name.c_str(), ext.c_str())) return true;
  }
  return false;
}

#include "callback.cc"
#include "editor_render_target.cc"

r32 EditorViewportCurrentScale();
rgg::Camera* EditorViewportCurrentCamera();

// We hand screw the scale to avoid avoid scaling in shaders. This gives tighter control of of pixely things.
// Idk if this is the correct move (instead of using camera scale and in shader). But This is the world
// we live in.
v2f ScaleVec2(const v2f& vec) {
  r32 scale = EditorViewportCurrentScale();
  return vec * scale;
}

r32 ScaleR32(r32 v) {
  r32 scale = EditorViewportCurrentScale();
  return v * scale;
}

s32 ScaleS32(s32 v) {
  r32 scale = EditorViewportCurrentScale();
  return (s32)(v * scale);
}

Rectf ScaleEditorRect(const Rectf& rect) {
  r32 scale = EditorViewportCurrentScale();
  Rectf dest = rect;
  if (scale != 1.f) {
    dest.width = dest.width * scale;
    dest.height = dest.height * scale;
  }
  dest.x -= dest.width / 2.f;
  dest.y -= dest.height / 2.f;
  return dest;
}

Rectf ScaleRect(const Rectf& rect) {
  r32 scale = EditorViewportCurrentScale();
  Rectf scaled_rect = rect;
  scaled_rect.x *= scale;
  scaled_rect.y *= scale;
  scaled_rect.width *= scale;
  scaled_rect.height *= scale;
  return scaled_rect;
}

struct SpriteAnimatorSelection {
  // 0 means selection has not started, 1 means the the first selection has been made, 2 means the final
  // selection has been made.
  s32 action = 0;
  v2f start_texcoord;
  v2f end_texcoord;
  Rectf TexRect() const {
    return math::OrientToAabb(math::MakeRect(start_texcoord, end_texcoord));
  }
  v2f start_world;
  v2f end_world;
  Rectf WorldRect() const {
    return math::OrientToAabb(math::MakeRect(start_world, end_world));
  }
  Rectf WorldRectScaled() const;
};

static SpriteAnimatorSelection kSpriteAnimatorSelection;

void EditorDebugMenuGrid(EditorGrid* grid) {
  if (ImGui::Button("-##cellw")) {
    --grid->cell_width;
  }
  ImGui::SameLine();
  if (ImGui::Button("+##cellw")) {
    ++grid->cell_width;
  }
  ImGui::SameLine();
  ImGui::SliderInt("cellw", &grid->cell_width, 1, 256);
  
  if (ImGui::Button("-##cellh")) {
    --grid->cell_height;
  }
  ImGui::SameLine();
  if (ImGui::Button("+##cellh")) {
    ++grid->cell_height;
  }
  ImGui::SameLine();
  ImGui::SliderInt("cellh", &grid->cell_height, 1, 256);

  if (ImGui::Button("-##offsetx")) {
    --grid->origin_offset.x;
  }
  ImGui::SameLine();
  if (ImGui::Button("+##offsetx")) {
    ++grid->origin_offset.x;
  }
  ImGui::SameLine();
  ImGui::SliderFloat("offsetx", &grid->origin_offset.x, -128.f, 128.f, "%.0f");

  if (ImGui::Button("-##offsety")) {
    --grid->origin_offset.y;
  }
  ImGui::SameLine();
  if (ImGui::Button("+##offsety")) {
    ++grid->origin_offset.y;
  }
  ImGui::SameLine();
  ImGui::SliderFloat("offsety", &grid->origin_offset.y, -128.f, 128.f, "%.0f");
}

void EditorRenderCrosshair(v2f point_scaled, const Rectf& view, const v4f& color = rgg::kRed) {
  rgg::RenderLine(v2f(view.Min().x, point_scaled.y),
                  v2f(view.Max().x, point_scaled.y), color);
  rgg::RenderLine(v2f(point_scaled.x, view.Min().y),
                  v2f(point_scaled.x, view.Max().y), color);
}

void EditorExit() {
  exit(0);
}

void EditorSetCurrent(EditorRenderTarget* render_target) {
  kEditor.current = render_target;
}

bool EditorShouldIgnoreFile(const char* filename) {
  s32 len = (s32)strlen(filename);
  if (filename[0] == '.') return true;
  else if (filename[len - 1] == '~') return true;
  return false;
}

void EditorFilesFrom(const char* dir) {
  std::vector<std::string> files;
  std::vector<std::string> dirs;
  filesystem::WalkDirectory(dir, [&files, &dir, &dirs](const char* filename, bool is_dir) {
    if (EditorShouldIgnoreFile(filename)) return;
    if (is_dir) {
      dirs.push_back(std::string(filename));
    } else {
      files.push_back(filesystem::JoinPath(dir, std::string(filename)));
    }
  });
  for (const std::string& d : dirs) {
    if (ImGui::TreeNode(d.c_str())) {
      char subdir[256] = {};
      if (dir[strlen(dir) - 1] == '*')
        strncat(subdir, dir, strlen(dir) - 1);
      else
        strcat(subdir, dir);
      strcat(subdir, d.c_str());
#ifdef _WIN32
      strcat(subdir, "\\*");
#else
      strcat(subdir, "/");
#endif
      EditorFilesFrom(subdir);
      ImGui::Unindent();
      ImGui::TreePop();
    }
  }
  ImGui::Indent();
  for (const std::string& file : files) {
    bool kChosen = false;
    std::string filename = filesystem::Filename(file.c_str());
    if (EditorCanLoadAsset(file)) {
      if (ImGui::Selectable(filename.c_str(), &kChosen)) {
        if (kEditor.current) {
          kEditor.current->OnFileSelected(GetAssetRelative(file));
        }
      }
    } else {
      ImGui::Text("%s", filename.c_str());
    }
  }
}

void EditorFileBrowserDefault() {
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  v2f wsize = window::GetWindowSize();
  ImGui::SetNextWindowSize(ImVec2((float)kExplorerWidth, (float)wsize.y * (2 / 5.f)));
  ImGui::SetNextWindowPos(ImVec2((float)kExplorerStart, 0.f), ImGuiCond_Always);
  ImGui::Begin("File Browser", nullptr, window_flags);
  char dir[256] = {};
#ifdef _WIN32
  strcat(dir, filesystem::GetWorkingDirectory());
  strcat(dir, "\\*");
#else
  strcat(dir, "./");
#endif
  EditorFilesFrom(dir);
  ImGui::End();
}

#include "sprite_animator.cc"
#include "game_viewer.cc"
#include "map_maker.cc"
#include "entity_creator.cc"

r32 EditorViewportCurrentScale() {
  if (kEditor.current) return kEditor.current->scale_;
  return 1.f;
}

Rectf SpriteAnimatorSelection::WorldRectScaled() const {
  r32 scale = EditorViewportCurrentScale();
  return math::OrientToAabb(math::MakeRect(start_world * scale, end_world * scale));
}

rgg::Camera* EditorViewportCurrentCamera() {
  if (kEditor.current) return kEditor.current->camera();
  return nullptr;
}

void EditorProcessEvent(const PlatformEvent& event) {
  switch (kEditor.mode) {
    case EDITOR_MODE_GAME: {
      EditorGameViewerProcessEvent(event);
    } break;
    case EDITOR_MODE_SPRITE_ANIMATOR: {
      EditorSpriteAnimatorProcessEvent(event);
    } break;
    case EDITOR_MODE_MAP_MAKER: {
      EditorMapMakerProcessEvent(event);
    } break;
    case EDITOR_MODE_ENTITY_CREATOR: {
      EditorEntityCreatorProcessEvent(event);
    } break;
  }

  switch(event.type) {
    case KEY_DOWN: {
      switch (event.key) {
        case KEY_ESC: {
          EditorExit();
        } break;
      }
    } break;
    case KEY_UP:
    case MOUSE_UP:
    case MOUSE_WHEEL:
    case NOT_IMPLEMENTED:
    default: break;
  }
}

void EditorFileBrowser() {
  switch (kEditor.mode) {
    case EDITOR_MODE_GAME: {
      EditorGameViewerFileBrowser();
    } break;
    case EDITOR_MODE_SPRITE_ANIMATOR: {
      EditorSpriteAnimatorFileBrowser();
    } break;
    case EDITOR_MODE_MAP_MAKER: {
      EditorMapMakerFileBrowser();
    } break;
    case EDITOR_MODE_ENTITY_CREATOR: {
      EditorEntityCreatorFileBrowser();
    } break;
  }
}


void EditorDebugMenu() {
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  v2f wsize = window::GetWindowSize();
  ImGui::SetNextWindowSize(ImVec2((float)kExplorerWidth, (float)wsize.y * (3 / 5.f)));
  ImGui::SetNextWindowPos(ImVec2((float)kExplorerStart, (float)wsize.y * (2 / 5.f)), ImGuiCond_Always);
  static const s32 kTabCount = 3;
  static const char* kTabs[kTabCount] = {
    "Contextual",
    "Diagnostics",
    "Textures"
  };
  static bool kOpened[kTabCount] = { true, true, true }; // Persistent user state
  ImGui::Begin("Debug", nullptr, window_flags);
  if (ImGui::BeginTabBar("Debug Tabs")) {
    for (s32 i = 0; i < kTabCount; ++i) {
      if (kOpened[i] && ImGui::BeginTabItem(kTabs[i], &kOpened[i], ImGuiTabItemFlags_None)) {
        if (i == 0) {
          switch (kEditor.mode) {
            case EDITOR_MODE_GAME: {
              EditorGameViewerDebug();
            } break;
            case EDITOR_MODE_SPRITE_ANIMATOR: {
              EditorSpriteAnimatorDebug();
            } break;
            case EDITOR_MODE_MAP_MAKER: {
              EditorMapMakerDebug();
            } break;
            case EDITOR_MODE_ENTITY_CREATOR: {
              EditorEntityCreatorDebug();
            } break;
          }
        } else if (i == 1) {
          if (kEditor.current) {
            const EditorCursor& cursor = kEditor.current->cursor();
            ImGui::Text("Cursor");
            ImGui::Text("  global screen             %.0f %.0f", cursor.global_screen.x, cursor.global_screen.y);
            if (cursor.is_in_viewport) {
              ImGui::Text("  world                     %.0f %.0f", cursor.world.x, cursor.world.y);
              ImGui::Text("  world scaled              %.0f %.0f", cursor.world_scaled.x, cursor.world_scaled.y);
              ImGui::Text("  world clamped             %.0f %.0f", cursor.world_clamped.x, cursor.world_clamped.y);
              ImGui::Text("  local screen              %.0f %.0f", cursor.local_screen.x, cursor.local_screen.y);
            } else {
              ImGui::Text("  world                     x x");
              ImGui::Text("  world scaled              x x");
              ImGui::Text("  world clamped             x x");
              ImGui::Text("  local screen              x x");
            }
            ImGui::NewLine();
          }
          rgg::Camera* camera = EditorViewportCurrentCamera();
          if (camera) {
            ImGui::Text("Camera");
            ImGui::Text("  pos     %.1f %.1f %.1f", camera->position.x, camera->position.y, camera->position.z);
            ImGui::Text("  dir     %.1f %.1f %.1f", camera->dir.x, camera->dir.y, camera->dir.z);
            ImGui::Text("  up      %.1f %.1f %.1f", camera->up.x, camera->up.y, camera->up.z);
            ImGui::Text("  vp      %.1f %.1f", camera->viewport.x, camera->viewport.y);
            ImGuiTextRect("  crect  ", kSpriteAnimator.GetCameraRectScaled());
            ImGuiTextRect("  crects ", kSpriteAnimator.GetCameraRect());
            ImGui::NewLine();
          }
    
          ImGui::Text("Game Loop");
          ImGui::Text("  runtime    %04.02fs", (r64)kGameState.game_time_usec / 1e6);
          ImGui::Text("  frametime  %04.02fus [%02.02f%%]",
                      StatsMean(&kGameStats), 100.f * StatsUnbiasedRsDev(&kGameStats));
        } else if (i == 2) {
          ImGui::Text("Cached Textures (%lu/%u)", rgg::kUsedTextureHandle, RGG_TEXTURE_MAX);
          ImGui::NewLine();
          rgg::IterateTextures([](const rgg::TextureHandle* handle) {
            ImGui::Text("Fullpath (%s)", handle->texture.file.c_str());
            ImGui::Text("  name    %s", filesystem::Filename(handle->texture.file).c_str());
            ImGui::Text("  id      %u", handle->id);
            ImGui::Text("  gl id   %u", handle->texture.reference);
            ImGui::Text("  dims    %.2f %.2f", handle->texture.width, handle->texture.height);
          });
        }
        ImGui::EndTabItem();
      }
    }
    ImGui::EndTabBar();
  }
  ImGui::End();
}

void EditorRenderViewport() {
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
  v2f wsize = window::GetWindowSize();
  r32 render_view_width = wsize.x - kExplorerWidth;
  ImVec2 imsize = ImVec2(render_view_width, wsize.y);
  // The viewport as it exists in the global bounds of the editor window.
  ImGuiStyle& style = ImGui::GetStyle();
  kEditor.render_viewport_in_editor = Rectf(
    kRenderViewStart + style.WindowPadding.x, 0.f, imsize.x, imsize.y);
  // The viewport as it exists in the local bounds of the viewport.
  kEditor.render_viewport = Rectf(0.f, 0.f, imsize.x, imsize.y - 39.f);
  ImGui::SetNextWindowSize(imsize);
  ImGui::SetNextWindowPos(ImVec2((float)kRenderViewStart, 0.f), ImGuiCond_Always);
  ImGui::Begin("Render Viewport", nullptr, window_flags);
  if (ImGui::BeginTabBar("Render Tabs")) {
    for (s32 i = 0; i < kViewportTabCount; ++i) {
      if (kOpened[i] && ImGui::BeginTabItem(kViewportTabs[i], &kOpened[i], ImGuiTabItemFlags_None)) {
        //ImGui::Text("Tab %s", kViewportTabs[i]);
        if (i == 0) {
          EditorGameViewerMain();
          kEditor.mode = EDITOR_MODE_GAME;
        } else if (i == 1) {
          EditorSpriteAnimatorMain();
          kEditor.mode = EDITOR_MODE_SPRITE_ANIMATOR;
        } else if (i == 2) {
          EditorMapMakerMain();
          kEditor.mode = EDITOR_MODE_MAP_MAKER;
        } else if (i == 3) {
          EditorEntityCreatorMain();
          kEditor.mode = EDITOR_MODE_ENTITY_CREATOR;
        }
        ImGui::EndTabItem();
      }
    }
    ImGui::EndTabBar();
  }
  ImGui::End();
}


void EditorMain() {
  EditorFileBrowser();
  EditorDebugMenu();
  EditorRenderViewport();
}
