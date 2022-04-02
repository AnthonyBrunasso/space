#pragma once

class MapMaker : public EditorRenderTarget {
public:
  void OnRender() override;
  void OnImGui() override;
  void OnFileSelected(const std::string& filename) override;
};

static MapMaker kMapMaker;

class MapMakerControl : public EditorRenderTarget {
  void OnRender() override;
  void OnImGui() override;
};

static MapMakerControl kMapMakerControl;

void MapMaker::OnRender() {
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4 imcolor = style.Colors[ImGuiCol_WindowBg];

  // Fill the background with imgui's background color to maintain beauty.
  rgg::RenderRectangle(GetCameraRectScaled(), v4f(imcolor.x, imcolor.y, imcolor.z, imcolor.w));

  RenderGrid(v4f(1.f, 1.f, 1.f, 0.2f));
  RenderAxis();
}

void MapMaker::OnImGui() {
  UpdateImguiPanelRect();
  ImGuiImage();
}

void MapMaker::OnFileSelected(const std::string& filename) {
  LOG(INFO, "MapMaker::%s", __func__);
}

void MapMakerControl::OnRender() {
}

void MapMakerControl::OnImGui() {
  ImGui::Begin("Map Maker Control");
  ImGui::Text("Hello!");
  ImGui::End();
}

void EditorMapMakerProcessEvent(const PlatformEvent& event) {
  kMapMaker.UpdateCursor();
}

void EditorMapMakerInitialize() {
  static bool do_once = true;
  if (!do_once) {
    return;
  }
  kMapMaker.Initialize((s32)kEditor.render_viewport.width, (s32)kEditor.render_viewport.height);
  do_once = false;
}

void EditorMapMakerMain() {
  EditorSetCurrent(&kMapMaker);
  EditorMapMakerInitialize();
  kMapMaker.Render();
  kMapMakerControl.Render();
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
