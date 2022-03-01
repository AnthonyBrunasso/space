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

struct EditorState {
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
static EditorState kEditorState;

static s32 kExplorerStart = 0;
static s32 kExplorerWidth = 300;

static s32 kRenderViewStart = 300;
//static s32 kRenderViewWidth = 600;

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
  Rectf renderable_edges = kEditorState.render_viewport;
  return ScaleEditorRect(renderable_edges);
}

EditorGrid ScaleGrid(const EditorGrid& grid) {
  r32 scale = EditorViewportCurrentScale();
  EditorGrid scaled_grid;
  scaled_grid.cell_width = grid.cell_width * scale;
  scaled_grid.cell_height = grid.cell_height * scale;
  scaled_grid.origin.x = grid.origin.x * scale;
  scaled_grid.origin.y = grid.origin.y * scale;
  return scaled_grid;
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
  Rectf viewport;
};

static AssetViewerSelection kAssetViewerSelection;

void EditorDebugMenuGrid() {
  ImGui::Text("World grid origin (%.1f %.1f)", kGrid.origin.x, kGrid.origin.y);
  ImGui::SliderInt("cellw", &kGrid.cell_width, 1, 64);
  ImGui::SliderInt("cellh", &kGrid.cell_height, 1, 64);
  if (kAssetViewerSelection.action == 2) {
    ImGui::Text("asset viewer %.1f %.1f %.1f %.1f", 
                kAssetViewerSelection.viewport.x,
                kAssetViewerSelection.viewport.y,
                kAssetViewerSelection.viewport.width,
                kAssetViewerSelection.viewport.height);
  }
}

#include "asset_viewer.cc"
#include "game_viewer.cc"

r32 EditorViewportCurrentScale() {
  switch (kEditorState.mode) {
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

void EditorExit() {
  exit(0);
}

rgg::Camera* EditorViewportCurrentCamera() {
  switch (kEditorState.mode) {
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
  kCursor.world_scaled = kCursor.local_screen - (kEditorState.render_viewport.Dims() / 2.f);
  r32 scale = EditorViewportCurrentScale();
  // To get the inverse world space we divide out the scale.
  kCursor.world = Roundf(kCursor.world_scaled / scale);
  rgg::Camera* camera = EditorViewportCurrentCamera();
  if (camera) {
    kCursor.world += (camera->position.xy() / scale);
    kCursor.world_scaled += camera->position.xy();
  }
  kCursor.is_in_viewport = math::PointInRect(kCursor.global_screen, kEditorState.render_viewport_in_editor);
  // Move the cursor into grid space in the world
  v2f cursor_relative = kCursor.world - kGrid.origin;
  // Clamp to nearest grid edge.
  Rectf rgrid;
  rgrid.x = roundf(cursor_relative.x - ((int)roundf(cursor_relative.x) % kGrid.cell_width));
  rgrid.y = roundf(cursor_relative.y - ((int)roundf(cursor_relative.y) % kGrid.cell_height));
  rgrid.x = cursor_relative.x < 0.f ? rgrid.x - kGrid.cell_width : rgrid.x;
  rgrid.y = cursor_relative.y < 0.f ? rgrid.y - kGrid.cell_height : rgrid.y;
  rgrid.width = kGrid.cell_width;
  rgrid.height = kGrid.cell_height;
  kCursor.world_clamped = Roundf(rgrid.NearestEdge(cursor_relative)) + kGrid.origin;
}

void EditorProcessEvent(const PlatformEvent& event) {
  // Update cursor state.
  EditorUpdateCursor();

  switch (kEditorState.mode) {
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
        case KEY_ARROW_UP: {
          rgg::Camera* camera = EditorViewportCurrentCamera();
          if (camera) {
            camera->position += v2f(0.f, ScaleR32(16.f));
          }
        } break;
        case KEY_ARROW_RIGHT: {
          rgg::Camera* camera = EditorViewportCurrentCamera();
          if (camera) {
            camera->position += v2f(ScaleR32(16.f), 0.f);
          }
        } break;
        case KEY_ARROW_DOWN: {
          rgg::Camera* camera = EditorViewportCurrentCamera();
          if (camera) {
            camera->position += v2f(0.f, ScaleR32(-16.f));
          }
        } break;
        case KEY_ARROW_LEFT: {
          rgg::Camera* camera = EditorViewportCurrentCamera();
          if (camera) {
            camera->position += v2f(ScaleR32(-16.f), 0.f);
          }
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
  filesystem::WalkDirectory(dir, [&files, &dirs](const char* filename, bool is_dir) {
    if (EditorShouldIgnoreFile(filename)) return;
    if (is_dir) dirs.push_back(std::string(filename));
    else files.push_back(std::string(filename));
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
    if (EditorCanLoadAsset(file)) {
      if (ImGui::Selectable(file.c_str(), &kChosen)) {
        //LOG(INFO, "Chose asset %s", file.c_str());
#ifdef _WIN32
        // Windows does dumb stuff with their API to recursively expand paths. So fix that here.
        char corrected_dir[256] = {};
        strncpy(corrected_dir, dir, strlen(dir) - 2);
        kAssetViewer.chosen_asset_path = filesystem::JoinPath(corrected_dir, file);
#else
        kAssetViewer.chosen_asset_path = filesystem::JoinPath(dir, file);
#endif
        kEditorState.mode = EDITOR_MODE_ASSET_VIEWER;
      }
    }
    else {
      ImGui::Text("%s", file.c_str());
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
  strcat(dir, filesystem::GetWorkingDirectory());
#ifdef _WIN32
  strcat(dir, "\\*");
#else
  strcat(dir, "/../");
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
  ImGui::SetNextWindowSize(ImVec2(kExplorerWidth, wsize.y * (2 / 5.f)));
  ImGui::SetNextWindowPos(ImVec2(kExplorerStart, (item_height + 1.f) + wsize.y * (3 / 5.f)), ImGuiCond_Always);
  static const s32 kTabCount = 2;
  static const char* kTabs[kTabCount] = {
    "Contextual",
    "System"
  };
  static bool kOpened[kTabCount] = { true, true }; // Persistent user state
  ImGui::Begin("Debug", nullptr, window_flags);
  if (ImGui::BeginTabBar("Debug Tabs")) {
    for (s32 i = 0; i < kTabCount; ++i) {
      if (kOpened[i] && ImGui::BeginTabItem(kTabs[i], &kOpened[i], ImGuiTabItemFlags_None)) {
        if (i == 0) {
          switch (kEditorState.mode) {
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
  kEditorState.render_viewport_in_editor = Rectf(
    kRenderViewStart + style.WindowPadding.x, 0.f, imsize.x - 15.f, imsize.y - 50.f);
  // The viewport as it exists in the local bounds of the viewport.
  kEditorState.render_viewport = Rectf(
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
          kEditorState.mode = EDITOR_MODE_GAME;
        }
        else if (i == 1) {
          EditorAssetViewerMain();
          kEditorState.mode = EDITOR_MODE_ASSET_VIEWER;
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
