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

  AnimSequence2d* anim() { return &running_anim2d_; }

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
  if (entity_.animation_size() > 0) {
    if (ImGui::TreeNode("Animations")) {
      static char** kTypeStrings = nullptr;
      if (!kTypeStrings) {
        const google::protobuf::EnumDescriptor* desc = proto::Entity2d_Animation_Type_descriptor();
        kTypeStrings = new char*[proto::Entity2d_Animation::Type_MAX + 1];
        for (s32 i = 0; i <= proto::Entity2d_Animation::Type_MAX; ++i) {
          std::string name = desc->FindValueByNumber(i)->name();
          kTypeStrings[i] = new char[name.size()];
          strcpy(kTypeStrings[i], name.data());
        }
      }
      for (s32 i = 0; i < entity_.animation_size();) {
        proto::Entity2d::Animation* proto_animation = entity_.mutable_animation(i);
        static char kTempStr[128];
        snprintf(kTempStr, 128, "x##%s", proto_animation->animation_file().c_str());
        if (ImGui::Button(kTempStr)) {
          entity_.mutable_animation()->SwapElements(i, entity_.animation_size() - 1);
          entity_.mutable_animation()->RemoveLast();
          continue;
        }
        ImGui::SameLine();
        if (ImGui::Selectable(proto_animation->animation_file().c_str())) {
          if (!AnimSequence2d::LoadFromProtoFile(
              proto_animation->animation_file().c_str(), &running_anim2d_)) {
            LOG(ERR, "Unable to load proto file %s", proto_animation->animation_file().c_str());
          }
          running_anim2d_.Start();
        }
        ImGui::Indent();
        proto::Entity2d_Animation_Type type = proto_animation->type();
        snprintf(kTempStr, 128, "<##%i", i);
        if (ImGui::Button(kTempStr)) {
          type = (proto::Entity2d_Animation_Type)((s32)type - 1);
          if (type < 0) type = (proto::Entity2d_Animation_Type)((s32)proto::Entity2d_Animation::Type_MAX);
        }
        ImGui::SameLine();
        snprintf(kTempStr, 128, ">##%i", i);
        if (ImGui::Button(kTempStr)) {
          type = (proto::Entity2d_Animation_Type)((s32)type + 1);
          if (type > proto::Entity2d_Animation::Type_MAX) type = (proto::Entity2d_Animation_Type)0;
        }
        ImGui::SameLine();
        snprintf(kTempStr, 128, "type##%s", proto_animation->animation_file().c_str());
        ImGui::Combo(kTempStr, (s32*)&type, kTypeStrings, proto::Entity2d_Animation::Type_MAX + 1);
        proto_animation->set_type(type);
        r32 alignment_x = proto_animation->alignment_x();
        snprintf(kTempStr, 128, "-##x%i", i);
        if (ImGui::Button(kTempStr)) {
          --alignment_x;
        }
        ImGui::SameLine();
        snprintf(kTempStr, 128, "+##x%i", i);
        if (ImGui::Button(kTempStr)) {
          ++alignment_x;
        }
        ImGui::SameLine();
        snprintf(kTempStr, 128, "alignx##%i", i);
        ImGui::SliderFloat(kTempStr, &alignment_x, -128.f, 128.f, "%.0f");
        r32 alignment_y = proto_animation->alignment_y();
        snprintf(kTempStr, 128, "-##y%i", i);
        if (ImGui::Button(kTempStr)) {
          --alignment_y;
        }
        ImGui::SameLine();
        snprintf(kTempStr, 128, "+##y%i", i);
        if (ImGui::Button(kTempStr)) {
          ++alignment_y;
        }
        ImGui::SameLine();
        snprintf(kTempStr, 128, "aligny##%i", i);
        ImGui::SliderFloat(kTempStr, &alignment_y, -128.f, 128.f, "%.0f");
        if (running_anim2d_.file_ == proto_animation->animation_file()) {
          running_anim2d_.SetAlignment(v2f(alignment_x, alignment_y));
        }
        proto_animation->set_alignment_x(alignment_x);
        proto_animation->set_alignment_y(alignment_y);
        ImGui::Unindent();
        ++i;
      }
      ImGui::TreePop();
    }
  }
  ImGui::Separator();
  static char kFullPath[256];
  ImGui::InputText("file", entity_name_, 128); 
  snprintf(kFullPath, 256, "gamedata/entities/%s.entity", entity_name_);
  ImGui::Text("%s", kFullPath);
  if (ImGui::Button("save")) {
    std::fstream fo(kFullPath, std::ios::binary | std::ios::out);
    entity_.SerializeToOstream(&fo);
    fo.close();
  }
  ImGui::SameLine();
  if (ImGui::Button("load")) {
  }
  ImGui::SameLine();
  if (ImGui::Button("clear")) {
  }
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

void EditorEntityCreatorProcessEvent(const PlatformEvent& event) {
  kEntityCreator.UpdateCursor();
  switch(event.type) {
    case MOUSE_WHEEL: {
      if (kEntityCreator.IsMouseInsideEditorSurface()) {
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
  static char kAnimationsDir[256] = {};
  if (kAnimationsDir[0] == 0) {
#ifdef _WIN32
    strcat(kAnimationsDir, filesystem::GetWorkingDirectory());
    strcat(kAnimationsDir, "\\gamedata\\animations\\*");
#else
    strcat(kAnimationsDir, "./gamedata/animations/");
#endif
  }
  if (ImGui::TreeNode("Animations")) {
    EditorEntityCreatorWalkAnimations(kAnimationsDir);
    ImGui::TreePop();
  }
  static char kEntitiesDir[256] = {};
  if (kEntitiesDir[0] == 0) {
#ifdef _WIN32
    strcat(kEntitiesDir, filesystem::GetWorkingDirectory());
    strcat(kEntitiesDir, "\\gamedata\\entities\\*");
#else
    strcat(kEntitiesDir, "./gamedata/entities/");
#endif
  }
  if (ImGui::TreeNode("Entities")) {
    EditorEntityCreatorWalkEntities(kEntitiesDir);
    ImGui::TreePop();
  }

  ImGui::End();
}
