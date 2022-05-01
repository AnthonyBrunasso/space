#pragma once

#include "game/main.cc"

void EditorGameViewerProcessEvent(const PlatformEvent& event) {
  GameProcessEvent(event);
}

void EditorGameViewerMain() {
  EditorSetCurrent(&kGame);
  kGame.Main();
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
