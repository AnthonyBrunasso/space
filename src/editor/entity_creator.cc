#pragma once

class EntityCreator : public EditorRenderTarget {
public:
  void OnInitialize() override;
  void OnRender() override;
  void OnImGui() override;

};

static EntityCreator kEntityCreator;

void EntityCreator::OnInitialize() {
}

void EntityCreator::OnRender() {
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4 imcolor = style.Colors[ImGuiCol_WindowBg];
  rgg::RenderRectangle(GetCameraRectScaled(), v4f(imcolor.x, imcolor.y, imcolor.z, imcolor.w));
  RenderGrid(v4f(1.f, 1.f, 1.f, 0.2f));
  RenderAxis();
}

void EntityCreator::OnImGui() {
  UpdateImguiPanelRect();
  ImGuiImage();
}

void EditorEntityCreatorProcessEvent(const PlatformEvent& event) {
}

void EditorEntityCreatorInitialize() {
  static bool do_once = true;
  if (!do_once) {
    return;
  }
  LOG(INFO, "Creating entity creator viewport with %i %i", (s32)kEditor.render_viewport.width, (s32)kEditor.render_viewport.height);
  kEntityCreator.Initialize((s32)kEditor.render_viewport.width, (s32)kEditor.render_viewport.height);
  do_once = false;
}

void EditorEntityCreatorMain() {
  EditorSetCurrent(&kEntityCreator);
  EditorEntityCreatorInitialize();
  kEntityCreator.Render();
}

void EditorEntityCreatorDebug() {
  EditorDebugMenuGrid(kEntityCreator.grid());
}
