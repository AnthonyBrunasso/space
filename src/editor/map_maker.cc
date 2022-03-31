#pragma once

void EditorMapMakerProcessEvent(const PlatformEvent& event) {
}

void EditorMapMakerMain() {
}

void EditorMapMakerDebug() {
  ImGui::Text("Game");
}

rgg::Camera* EditorMapMakerCamera() {
  return nullptr;
}

r32 EditorMapMakerScale() {
  return 1.f;
}
