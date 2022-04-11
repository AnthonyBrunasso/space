#pragma once

#include <google/protobuf/descriptor.h>

#include "entity.pb.h"

class EntityCreator : public EditorRenderTarget {
public:
  void OnInitialize() override;
  void OnRender() override;
  void OnImGui() override;
};

static EntityCreator kEntityCreator;

class EntityCreatorControl {
public:
  void ImGui();
};

static EntityCreatorControl kEntityCreatorControl;

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

void EntityCreatorControl::ImGui() {
  ImGui::Begin("Entity Creator Control");
  /*proto::Entity2d entity;
  const google::protobuf::Descriptor* desc = entity.GetDescriptor();
  for (s32 i = 0; i < desc->field_count(); ++i) {
    const google::protobuf::FieldDescriptor* field = desc->field(i);
    ImGui::Text("%s", field->name().c_str());
  }*/
  ImGui::Separator();
  static char kFullPath[256];
  static char kEntityName[128];
  ImGui::InputText("file", kEntityName, 128); 
  snprintf(kFullPath, 256, "gamedata/entities/%s.entity", kEntityName);
  ImGui::Text("%s", kFullPath);
  if (ImGui::Button("save")) {
  }
  ImGui::SameLine();
  if (ImGui::Button("load")) {
  }
  ImGui::SameLine();
  if (ImGui::Button("clear")) {
  }
  ImGui::End();
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
  kEntityCreatorControl.ImGui();
}

void EditorEntityCreatorDebug() {
  EditorDebugMenuGrid(kEntityCreator.grid());
}

void EditorEntityCreatorFileBrowser() {
  //EditorFileBrowserDefault();
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  v2f wsize = window::GetWindowSize();
  ImGui::SetNextWindowSize(ImVec2((float)kExplorerWidth, (float)wsize.y * (2 / 5.f)));
  ImGui::SetNextWindowPos(ImVec2((float)kExplorerStart, 0.f), ImGuiCond_Always);
  ImGui::Begin("Entity Components", nullptr, window_flags);
  char dir[256] = {};
#ifdef _WIN32
  strcat(dir, filesystem::GetWorkingDirectory());
  strcat(dir, "\\gamedata\\animations\\*");
#else
  strcat(dir, "./gamedata/animations/");
#endif
  if (ImGui::TreeNode("Animations")) {
    ImGui::Indent();
    filesystem::WalkDirectory(dir, [](const char* filename, bool is_dir) {
      if (strcmp(filename, ".") == 0) return;
      if (strcmp(filename, "..") == 0) return;
      if (ImGui::Selectable(filename)) {
        LOG(INFO, "Animation %s selected", filename);
      }
    });
    ImGui::Unindent();
    ImGui::TreePop();
  }
  ImGui::End();
}
