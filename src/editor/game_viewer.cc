#pragma once

// Uncomment for different games...

//#include "platformer/main.cc"
#include "sim/main.cc"

void EditorGameViewerInitialize() {
  static bool do_once = true;
  if (do_once) {
    kGame.render_physics_ = true;
    do_once = false;
  }
}

void EditorGameViewerProcessEvent(const PlatformEvent& event) {
  GameProcessEvent(event);
}

void EditorGameViewerMain() {
  EditorSetCurrent(&kGame);
  EditorGameViewerInitialize();
  kGame.Main();
}

void EditorGameViewerDebug() {
  //ImGui::Text("Game");
  ImGui::Checkbox("render aabb", &kGame.render_aabb_);
  ImGui::Checkbox("render physics", &kGame.render_physics_);
}

r32 EditorGameViewerScale() {
  return 1.f;
}

void EditorGameViewerFileBrowser() {
  EditorFileBrowserDefault();
}
