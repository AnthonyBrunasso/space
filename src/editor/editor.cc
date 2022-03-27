#pragma once


// ImGui
//    * Top left is origin.

enum EditorMode {
  EDITOR_MODE_GAME = 0,
  EDITOR_MODE_ASSET_VIEWER = 1,
};

static const s32 kViewportTabCount = 2;

static const char* kViewportTabs[kViewportTabCount] = {
  "Game",
  "Asset"
};

static bool kOpened[kViewportTabCount] = { true, true };

struct Editor {
  u64 frame_rate = 60;
  window::CreateInfo window_create_info;
  platform::Clock clock;
  Rectf render_viewport_in_editor;
  Rectf render_viewport;
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
  // x-y coordinate of the nearest edge given by kGrid.
  v2f world_clamped;
  // x-y coordinates of the cursor in the game / asset viewer, assuming bottom left is origin.
  v2f local_screen;
  // True when the cursor is in the game / asset viewer.
  bool is_in_viewport = false;
};

static EditorCursor kCursor;
static EditorGrid kGrid;
static Editor kEditor;

static s32 kExplorerStart = 0;
static s32 kExplorerWidth = 300;

static s32 kRenderViewStart = 300;

#include "editor_render_target.cc"

r32 EditorViewportCurrentScale();

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
  return v * scale;
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

Rectf RectToWorld(const Rectf& rect) {
  Rectf dest = rect;
  dest.x -= dest.width / 2.f;
  dest.y -= dest.height / 2.f;
  return dest;
}

Rectf ScaleEditorViewport() {
  Rectf renderable_edges = kEditor.render_viewport;
  return ScaleEditorRect(renderable_edges);
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

struct AssetViewerSelection {
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

static AssetViewerSelection kAssetViewerSelection;

void EditorDebugMenuGrid() {
  ImGui::SliderInt("cellw", &kGrid.cell_width, 1, 128);
  ImGui::SliderInt("cellh", &kGrid.cell_height, 1, 128);
  ImGui::SliderFloat("offsetx", &kGrid.origin_offset.x, -64.f, 64.f, "%.0f");
  ImGui::SliderFloat("offsety", &kGrid.origin_offset.y, -64.f, 64.f, "%.0f");
}

r32 __get_grid_line_color(s32 alpha_num, s32 alpha_1, s32 alpha_2, s32 alpha_3) {
  if (alpha_num == alpha_1) return .1f;
  if (alpha_num == alpha_2) return .2f;
  if (alpha_num == alpha_3) return .4f;
  return .1f;
}

void EditorRenderGrid(v2f start_scaled, const EditorGrid& grid, v4f color) {
  const Rectf& view_rect = ScaleEditorViewport();
  s32 scaled_width = ScaleS32(grid.cell_width);
  s32 scaled_height = ScaleS32(grid.cell_height);
  assert(scaled_width != 0 && scaled_height != 0);
  s32 alpha_1_width = scaled_width;
  s32 alpha_2_width = alpha_1_width * 2;
  s32 alpha_3_width = alpha_2_width * 2;
  s32 alpha_1_height = scaled_height;
  s32 alpha_2_height = alpha_1_height * 2;
  s32 alpha_3_height = alpha_2_height * 2;
  // Draw lines right
  s32 alpha_num = 0;
  for (r32 start_x = start_scaled.x; start_x <= view_rect.Max().x; start_x += scaled_width) {
    color.w = __get_grid_line_color(alpha_num, alpha_1_width, alpha_2_width, alpha_3_width);
    rgg::RenderLine(v2f(start_x, view_rect.Min().y), v2f(start_x, view_rect.Max().y), color);
    alpha_num += scaled_width;
    if (alpha_num > alpha_3_width) alpha_num = alpha_1_width;
  }
  // Draw lines left
  alpha_num = alpha_1_width;
  for (r32 start_x = start_scaled.x - scaled_width; start_x >= view_rect.Min().x; start_x -= scaled_width) {
    color.w = __get_grid_line_color(alpha_num, alpha_1_width, alpha_2_width, alpha_3_width);
    rgg::RenderLine(v2f(start_x, view_rect.Min().y), v2f(start_x, view_rect.Max().y), color);
    alpha_num += scaled_width;
    if (alpha_num > alpha_3_width) alpha_num = alpha_1_width;
  }
  // Draw lines up
  alpha_num = 0;
  for (r32 start_y = start_scaled.y; start_y <= view_rect.Max().y; start_y += scaled_height) {
    color.w = __get_grid_line_color(alpha_num, alpha_1_height, alpha_2_height, alpha_3_height);
    rgg::RenderLine(v2f(view_rect.Min().x, start_y), v2f(view_rect.Max().x, start_y), color);
    alpha_num += scaled_height;
    if (alpha_num > alpha_3_height) alpha_num = alpha_1_height;
  }
  // Draw lines down
  alpha_num = alpha_1_height;
  for (r32 start_y = start_scaled.y - scaled_height; start_y >= view_rect.Min().y; start_y -= scaled_height) {
    color.w = __get_grid_line_color(alpha_num, alpha_1_height, alpha_2_height, alpha_3_height);
    rgg::RenderLine(v2f(view_rect.Min().x, start_y), v2f(view_rect.Max().x, start_y), color);
    alpha_num += scaled_height;
    if (alpha_num > alpha_3_height) alpha_num = alpha_1_height;
  }
}

void EditorRenderAxis(v2f origin_scaled, const Rectf& view_rect) {
  rgg::RenderLine(v2f(origin_scaled.x, view_rect.Min().y),
                  v2f(origin_scaled.x, view_rect.Max().y),
                  v4f(0.f, 1.f, 0.f, 0.5f));
  rgg::RenderLine(v2f(view_rect.Min().x, origin_scaled.y),
                  v2f(view_rect.Max().x, origin_scaled.y),
                  v4f(0.f, 0.f, 1.f, 0.5f));
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

#include "asset_viewer.cc"
#include "game_viewer.cc"

r32 EditorViewportCurrentScale() {
  switch (kEditor.mode) {
    case EDITOR_MODE_GAME: {
      return EditorGameViewerScale();
    } break;
    case EDITOR_MODE_ASSET_VIEWER: {
      return EditorAssetViewerScale();
    } break;
  }
  return 1.f;
}

Rectf AssetViewerSelection::WorldRectScaled() const {
  r32 scale = EditorViewportCurrentScale();
  return math::OrientToAabb(math::MakeRect(start_world * scale, end_world * scale));
}

rgg::Camera* EditorViewportCurrentCamera() {
  switch (kEditor.mode) {
    case EDITOR_MODE_GAME: {
      return EditorGameViewerCamera();
    } break;
    case EDITOR_MODE_ASSET_VIEWER: {
      return EditorAssetViewerCamera();
    } break;
  }
  
  return nullptr;
}

void EditorUpdateCursor() {
  v2f cursor = window::GetCursorPosition();
  kCursor.global_screen = cursor;
  ImGuiStyle& style = ImGui::GetStyle();
  kCursor.local_screen = v2f(cursor.x - kExplorerWidth - style.WindowPadding.x, cursor.y);
  // User sees a scaled version of the world therefore a cursor placed in that space is in scaled world
  // space
  kCursor.world_scaled = kCursor.local_screen - (kEditor.render_viewport.Dims() / 2.f);
  r32 scale = EditorViewportCurrentScale();
  // To get the inverse world space we divide out the scale.
  kCursor.world = Roundf(kCursor.world_scaled / scale);
  rgg::Camera* camera = EditorViewportCurrentCamera();
  if (camera) {
    kCursor.world += (camera->position.xy() / scale);
    kCursor.world_scaled += camera->position.xy();
  }
  kCursor.is_in_viewport = math::PointInRect(kCursor.global_screen, kEditor.render_viewport_in_editor);
  // Move the cursor into grid space in the world
  v2f cursor_relative = kCursor.world - kGrid.GetOrigin();
  // Clamp to nearest grid edge.
  Rectf rgrid;
  rgrid.x = roundf(cursor_relative.x - ((int)roundf(cursor_relative.x) % kGrid.cell_width));
  rgrid.y = roundf(cursor_relative.y - ((int)roundf(cursor_relative.y) % kGrid.cell_height));
  rgrid.x = cursor_relative.x < 0.f ? rgrid.x - kGrid.cell_width : rgrid.x;
  rgrid.y = cursor_relative.y < 0.f ? rgrid.y - kGrid.cell_height : rgrid.y;
  rgrid.width = kGrid.cell_width;
  rgrid.height = kGrid.cell_height;
  kCursor.world_clamped = Roundf(rgrid.NearestEdge(cursor_relative)) + kGrid.GetOrigin();
}

void EditorProcessEvent(const PlatformEvent& event) {
  // Update cursor state.
  EditorUpdateCursor();

  switch (kEditor.mode) {
    case EDITOR_MODE_GAME: {
      EditorGameViewerProcessEvent(event);
    } break;
    case EDITOR_MODE_ASSET_VIEWER: {
      EditorAssetViewerProcessEvent(event);
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
  }
}

void EditorMainMenuFile() {
  if (ImGui::MenuItem("New")) {
  }
  if (ImGui::MenuItem("Open", "Ctrl+O")) {
  }
}

bool EditorShouldIgnoreFile(const char* filename) {
  int len = strlen(filename);
  if (filename[0] == '.') return true;
  else if (filename[len - 1] == '~') return true;
  return false;
}

void EditorFilesFrom(const char* dir) {
  std::vector<std::string> files;
  std::vector<std::string> dirs;
  filesystem::WalkDirectory(dir, [&files, &dir, &dirs](const char* filename, bool is_dir) {
    if (EditorShouldIgnoreFile(filename)) return;
    if (is_dir) dirs.push_back(std::string(filename));
    else files.push_back(filesystem::JoinPath(dir, std::string(filename)));
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
        //LOG(INFO, "Chose asset %s", file.c_str());
        kAssetViewer.chosen_asset_path_ = file;
        kEditor.mode = EDITOR_MODE_ASSET_VIEWER;
      }
    }
    else {
      ImGui::Text("%s", filename.c_str());
    }
  }
}

void EditorFileBrowser() {
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  v2f wsize = window::GetWindowSize();
  float item_height = ImGui::GetTextLineHeightWithSpacing();
  ImGui::SetNextWindowSize(ImVec2(kExplorerWidth, wsize.y * (3 / 5.f)));
  ImGui::SetNextWindowPos(ImVec2(kExplorerStart, item_height + 1.f), ImGuiCond_Always);
  ImGui::Begin("File Browser", nullptr, window_flags);
  char dir[256] = {};
#ifdef _WIN32
  strcat(dir, "\\*");
#else
  strcat(dir, "./");
#endif
  EditorFilesFrom(dir);
  ImGui::End();
}


void EditorDebugMenu() {
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  v2f wsize = window::GetWindowSize();
  float item_height = ImGui::GetTextLineHeightWithSpacing();
  ImGui::SetNextWindowSize(ImVec2(kExplorerWidth, wsize.y * (2 / 5.f) - item_height));
  ImGui::SetNextWindowPos(ImVec2(kExplorerStart, wsize.y * (3 / 5.f) + item_height), ImGuiCond_Always);
  static const s32 kTabCount = 3;
  static const char* kTabs[kTabCount] = {
    "Contextual",
    "System",
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
            case EDITOR_MODE_ASSET_VIEWER: {
              EditorAssetViewerDebug();
            } break;
          }
        }
        else if (i == 1) {
          ImGui::Text("Cursor");
          ImGui::Text("  global screen             %.0f %.0f",
                      kCursor.global_screen.x, kCursor.global_screen.y);
          if (kCursor.is_in_viewport) {
            ImGui::Text("  world                     %.0f %.0f",
                        kCursor.world.x, kCursor.world.y);
            ImGui::Text("  world scaled              %.0f %.0f",
                        kCursor.world_scaled.x, kCursor.world_scaled.y);
            ImGui::Text("  world clamped             %.0f %.0f",
                        kCursor.world_clamped.x, kCursor.world_clamped.y);
            ImGui::Text("  local screen              %.0f %.0f",
                        kCursor.local_screen.x, kCursor.local_screen.y);
          }
          else {
            ImGui::Text("  world                     x x");
            ImGui::Text("  world scaled              x x");
            ImGui::Text("  world clamped             x x");
            ImGui::Text("  local screen              x x");
          }
          ImGui::NewLine();
          rgg::Camera* camera = EditorViewportCurrentCamera();
          if (camera) {
            ImGui::Text("Camera");
            ImGui::Text("  pos    %.1f %.1f %.1f", camera->position.x, camera->position.y, camera->position.z);
            ImGui::Text("  dir    %.1f %.1f %.1f", camera->dir.x, camera->dir.y, camera->dir.z);
            ImGui::Text("  up     %.1f %.1f %.1f", camera->up.x, camera->up.y, camera->up.z);
            ImGui::Text("  vp     %.1f %.1f", camera->viewport.x, camera->viewport.y);
            ImGui::NewLine();
          }
          EditorDebugMenuGrid();
        }
        else if (i == 2) {
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

void EditorMainMenuBar() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      EditorMainMenuFile();
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}

void EditorRenderViewport() {
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
  v2f wsize = window::GetWindowSize();
  float item_height = ImGui::GetTextLineHeightWithSpacing();
  r32 render_view_width = wsize.x - kExplorerWidth;
  ImVec2 imsize = ImVec2(render_view_width, wsize.y);
  // The viewport as it exists in the global bounds of the editor window.
  ImGuiStyle& style = ImGui::GetStyle();
  kEditor.render_viewport_in_editor = Rectf(
    kRenderViewStart + style.WindowPadding.x, 0.f, imsize.x - 15.f, imsize.y - 50.f);
  // The viewport as it exists in the local bounds of the viewport.
  kEditor.render_viewport = Rectf(
    0.f, 0.f, imsize.x - 15.f, imsize.y - 50.f);
  ImGui::SetNextWindowSize(imsize);
  ImGui::SetNextWindowPos(ImVec2(kRenderViewStart, item_height + 1.f), ImGuiCond_Always);
  ImGui::Begin("Render Viewport", nullptr, window_flags);
  if (ImGui::BeginTabBar("Render Tabs")) {
    for (s32 i = 0; i < kViewportTabCount; ++i) {
      if (kOpened[i] && ImGui::BeginTabItem(kViewportTabs[i], &kOpened[i], ImGuiTabItemFlags_None)) {
        //ImGui::Text("Tab %s", kViewportTabs[i]);
        if (i == 0) {
          EditorGameViewerMain();
          kEditor.mode = EDITOR_MODE_GAME;
        }
        else if (i == 1) {
          EditorAssetViewerMain();
          kEditor.mode = EDITOR_MODE_ASSET_VIEWER;
        }
        ImGui::EndTabItem();
      }
    }
    ImGui::EndTabBar();
  }
  ImGui::End();
}


void EditorMain() {
  EditorMainMenuBar();
  EditorFileBrowser();
  EditorDebugMenu();
  EditorRenderViewport();
}
