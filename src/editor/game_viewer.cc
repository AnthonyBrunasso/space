#pragma once

void EditorGameViewerProcessEvent(const PlatformEvent& event) {
}

void EditorGameViewerMain() {
}

void EditorGameViewerDebug() {
  ImGui::Text("Game");
}

rgg::Camera* EditorGameViewerCamera() {
  return nullptr;
}

r32 EditorGameViewerScale() {
  return 1.f;
}