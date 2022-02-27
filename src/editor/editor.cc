#pragma once

// ImGui
//    * Top left is origin.

struct EditorState {
  u64 frame_rate = 60;
  window::CreateInfo window_create_info;
  platform::Clock clock;
  v2f render_dims;
};

static EditorState kEditorState;

static s32 kExplorerStart = 0;
static s32 kExplorerWidth = 300;

static s32 kRenderViewStart = 300;
//static s32 kRenderViewWidth = 600;

// This gives the renderable rect at the nearest point that a pixel won't be clipped out. It's a bit of a hack
// because basically if you try to render from 0, 0 to 0, view_height you won't see the line, likely due because
// it's clipped out of the texture.
Rectf EditorRenderableViewRect() {
  return math::MakeRect(
      v2f(-kEditorState.render_dims.x / 2.f + 0.01f, -kEditorState.render_dims.y / 2.f + 0.01f),
      v2f(kEditorState.render_dims.x / 2.f - 0.01f, kEditorState.render_dims.y / 2.f - 0.01f));
}

#include "asset_viewer.cc"
#include "game_viewer.cc"

void EditorExit() {
  exit(0);
}

void EditorProcessEvent(PlatformEvent& event) {
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
    ImGui::Text("%s", file.c_str());
  }
}

void EditorFileBrowser() {
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  v2f wsize = window::GetWindowSize();
  float item_height = ImGui::GetTextLineHeightWithSpacing();
  ImGui::SetNextWindowSize(ImVec2(kExplorerWidth, wsize.y));
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
  v2f wsize = window::GetWindowSize();
  float item_height = ImGui::GetTextLineHeightWithSpacing();
  r32 render_view_width = wsize.x - kExplorerWidth;
  ImVec2 imsize = ImVec2(render_view_width, wsize.y);
  kEditorState.render_dims.x = imsize.x - 15.f;
  kEditorState.render_dims.y = imsize.y - 50.f;
  ImGui::SetNextWindowSize(imsize);
  ImGui::SetNextWindowPos(ImVec2(kRenderViewStart, item_height + 1.f), ImGuiCond_Always);
  static const s32 kTabCount = 2;
  static const char* kTabs[kTabCount] = {
    "Game",
    "Asset"
  };
  static bool kOpened[kTabCount] = { true, true }; // Persistent user state
  ImGui::Begin("Render Viewport", nullptr, window_flags);
  if (ImGui::BeginTabBar("Render Tabs")) {
    for (s32 i = 0; i < kTabCount; ++i) {
      if (kOpened[i] && ImGui::BeginTabItem(kTabs[i], &kOpened[i], ImGuiTabItemFlags_None)) {
        //ImGui::Text("Tab %s", kTabs[i]);
        if (i == 0) EditorGameViewerMain();
        else if (i == 1) EditorAssetViewerMain();
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
  EditorRenderViewport();
}
