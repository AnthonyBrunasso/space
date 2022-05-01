#pragma once

#include "game/main.cc"

class GameViewer : public EditorRenderTarget {
public:
  void OnInitialize() override;
  void OnRender() override;
  void OnImGui() override;
  void OnFileSelected(const std::string& filename) override;
};

static GameViewer kGameViewer;

void GameViewer::OnInitialize() {
}

void GameViewer::OnRender() {
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4 imcolor = style.Colors[ImGuiCol_WindowBg];
  // Fill the background with imgui's background color to maintain beauty.
  rgg::RenderRectangle(GetCameraRectScaled(), v4f(imcolor.x, imcolor.y, imcolor.z, imcolor.w));

  GameRender();
}

void GameViewer::OnImGui() {
  ImGuiImage();
}

void GameViewer::OnFileSelected(const std::string& filename) {
  if (filesystem::HasExtension(filename, "map")) {
    GameLoadMap(filename);
  }
}

void EditorGameViewerProcessEvent(const PlatformEvent& event) {
  kGameViewer.UpdateCursor();
  GameProcessEvent(event);
}

void EditorGameInitialize() {
  static bool do_once = true;
  if (!do_once) {
    return;
  }
  kGameViewer.Initialize((s32)kEditor.render_viewport.width, (s32)kEditor.render_viewport.height);
  GameInitialize();
  do_once = false;
}

void EditorGameViewerMain() {
  EditorSetCurrent(&kGameViewer);
  EditorGameInitialize();
  GameMain();
  kGameViewer.Render();
}

void EditorGameViewerDebug() {
  ImGui::Text("Game");
}

r32 EditorGameViewerScale() {
  return 1.f;
}

void EditorGameViewerFileBrowser() {
  EditorFileBrowserDefault();
}
