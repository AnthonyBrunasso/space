#pragma once

#include <google/protobuf/descriptor.h>

#include "entity.pb.h"

class EntityCreator : public EditorRenderTarget {
public:
  void OnInitialize() override;
  void OnRender() override;
  void OnImGui() override;
  void ChangeScale(r32 delta);
};

static EntityCreator kEntityCreator;

class EntityCreatorControl {
public:
  void ImGui();
  void SelectAnimation(const char* filename);
  void SelectEntity(const char* filename);
  bool IsMouseInside();

  AnimSequence2d* anim() { return &running_anim2d_; }

  Rectf imgui_panel_rect_;
  proto::Entity2d entity_;
  AnimSequence2d running_anim2d_;
  char entity_name_[128];
};

static EntityCreatorControl kEntityCreatorControl;

void EntityCreator::OnInitialize() {
}

void EntityCreator::OnRender() {
  AnimSequence2d* anim = kEntityCreatorControl.anim();
  
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4 imcolor = style.Colors[ImGuiCol_WindowBg];
  rgg::RenderRectangle(GetCameraRectScaled(), v4f(imcolor.x, imcolor.y, imcolor.z, imcolor.w));

  if (!anim->IsEmpty()) {
    anim->Update();
    const AnimFrame2d& aframe = anim->CurrentFrame();
    const rgg::Texture* texture = aframe.GetTexture();
    Rectf dest_rect = Scale(Rectf(anim->alignment(), aframe.src_rect().Dims()));
    rgg::RenderTexture(*texture, aframe.src_rect(), dest_rect);
  }

  RenderGrid(v4f(1.f, 1.f, 1.f, 0.2f));
  RenderAxis();
}

void EntityCreator::OnImGui() {
  UpdateImguiPanelRect();
  ImGuiImage();
}

void EntityCreator::ChangeScale(r32 delta) {
  if (scale_ + delta > 0.f && scale_ + delta <= 15.f)
    scale_ += delta;
}

void EntityCreatorControl::ImGui() {
  ImGui::Begin("Entity Creator Control");
  EntityEditData entity_data;
  entity_data.grid = kEntityCreator.grid();
  entity_data.anim_sequence = &running_anim2d_;
  EntityEdit(entity_, &entity_data);
  ImGui::Separator();
  static char kFullPath[256];
  ImGui::InputText("file", entity_name_, 128); 
  snprintf(kFullPath, 256, "gamedata/entities/%s.entity", entity_name_);
  ImGui::Text("%s", kFullPath);
  if (ImGui::Button("save")) {
    std::fstream fo(kFullPath, std::ios::binary | std::ios::out);
    entity_.set_blueprint(kFullPath);
    LOG(INFO, "Saving %s", entity_.DebugString().c_str());
    entity_.SerializeToOstream(&fo);
    fo.close();
  }
  ImGui::SameLine();
  if (ImGui::Button("load")) {
  }
  ImGui::SameLine();
  if (ImGui::Button("clear")) {
  }
  GetImGuiPanelRect(&imgui_panel_rect_);
  ImGui::End();
}

void EntityCreatorControl::SelectAnimation(const char* filename) {
  proto::Entity2d::Animation* proto_animation = entity_.add_animation();
  proto_animation->set_animation_file(filename);
  if (!AnimSequence2d::LoadFromProtoFile(filename, &running_anim2d_)) {
    LOG(ERR, "Failed to load animation %s", filename);
  }
  running_anim2d_.Start();
}

void EntityCreatorControl::SelectEntity(const char* filename) {
  std::fstream inp(filename, std::ios::in | std::ios::binary);
  if (!entity_.ParseFromIstream(&inp)) {
    LOG(ERR, "Failed loading proto file %s", filename);
  } 
  if (!EntityLoadIdleAnimation(entity_, &running_anim2d_)) {
    LOG(INFO, "Entity %s has no idle animation.", filename);
  }
}

bool EntityCreatorControl::IsMouseInside() {
  return math::PointInRect(kEntityCreator.cursor().global_screen, imgui_panel_rect_);
}

void EditorEntityCreatorProcessEvent(const PlatformEvent& event) {
  kEntityCreator.UpdateCursor();
  switch(event.type) {
    case MOUSE_WHEEL: {
      if (kEntityCreator.IsMouseInsideEditorSurface() && !kEntityCreatorControl.IsMouseInside()) {
        if (event.wheel_delta > 0) {
          kEntityCreator.ChangeScale(1.f);
        } else if (event.wheel_delta < 0) {
          kEntityCreator.ChangeScale(-1.f);
        }
      }
    } break;
    case MOUSE_UP:
    case NOT_IMPLEMENTED:
    default: break;
  }
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
  if (ImGui::Button("-##scale")) {
    kEntityCreator.scale_--;
  }
  ImGui::SameLine();
  if (ImGui::Button("+##scale")) {
    kEntityCreator.scale_++;
  }
  kEntityCreator.scale_ = CLAMPF(kEntityCreator.scale_, 1.f, 15.f);
  ImGui::SameLine();
  ImGui::SliderFloat("scale", &kEntityCreator.scale_, 1.f, 15.f, "%.0f", ImGuiSliderFlags_None);
}

void EditorEntityCreatorWalkAnimations(const char* dir) {
  filesystem::WalkDirectory(dir, [dir](const char* filename, bool is_dir) {
    if (strcmp(filename, ".") == 0) return;
    if (strcmp(filename, "..") == 0) return;
    if (is_dir) {
      static char kTraverseInto[256];
      strcpy(kTraverseInto, dir);
      strcat(kTraverseInto, filename);
#ifdef _WIN32
      strcat(kTraverseInto, "\\*");
#else
      strcat(kTraverseInto, "/");
#endif
      EditorEntityCreatorWalkAnimations(kTraverseInto);
    } else if (ImGui::Selectable(filename) && filesystem::HasExtension(filename, "anim")) {
      kEntityCreatorControl.SelectAnimation(filesystem::JoinPath(dir, filename).c_str());
    }
  });
}

void EditorEntityCreatorWalkEntities(const char* dir) {
  filesystem::WalkDirectory(dir, [dir](const char* filename, bool is_dir) {
    if (strcmp(filename, ".") == 0) return;
    if (strcmp(filename, "..") == 0) return;
    if (is_dir) {
      static char kTraverseInto[256];
      strcpy(kTraverseInto, dir);
      strcat(kTraverseInto, filename);
#ifdef _WIN32
      strcat(kTraverseInto, "\\*");
#else
      strcat(kTraverseInto, "/");
#endif
      EditorEntityCreatorWalkEntities(filename);
    } else if (ImGui::Selectable(filename) && filesystem::HasExtension(filename, "entity")) {
      kEntityCreatorControl.SelectEntity(filesystem::JoinPath(dir, filename).c_str());
    }
  });
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
  
  if (ImGui::TreeNode("Animations")) {
    EditorEntityCreatorWalkAnimations(kAnimationsDir);
    ImGui::TreePop();
  }
  
  if (ImGui::TreeNode("Entities")) {
    EditorEntityCreatorWalkEntities(kEntitiesDir);
    ImGui::TreePop();
  }

  ImGui::End();
}
