#pragma once

enum class EntityExclusionBitfield {
  kLocation = 1 << 0,
  kAnimations = 1 << 1,
};

struct EntityEditData {
  EntityExclusionBitfield exclusion;
  EditorGrid* grid = nullptr;
  AnimSequence2d* anim_sequence = nullptr;
};

void EntityEdit(proto::Entity2d& entity, EntityEditData* edit_data) {
  static char kTempStr[128];
  static char** kEntityTypeStrings = nullptr;
  if (!kEntityTypeStrings) {
    CreateProtoEnumStrings(kEntityTypeStrings, proto::Entity2d_Type_descriptor());
  }
  proto::Entity2d::Type entity_type = entity.type();
  ImGui::Combo("entity_type", (s32*)&entity_type, kEntityTypeStrings, proto::Entity2d::Type_MAX + 1);
  entity.set_type(entity_type);
  r32 x = entity.location().x();
  r32 y = entity.location().y();
  if (ImGui::Button("-##location_x")) {
    x -= edit_data->grid->cell_width;
  }
  ImGui::SameLine();
  if (ImGui::Button("+##location_x")) {
    x += edit_data->grid->cell_width;
  }
  ImGui::SameLine();
  ImGui::SliderFloat("x##location", &x, -2048.f, 2048.f, "%.f");
  if (ImGui::Button("-##location_y")) {
    y -= edit_data->grid->cell_height;
  }
  ImGui::SameLine();
  if (ImGui::Button("+##location_y")) {
    y += edit_data->grid->cell_height;
  }
  ImGui::SameLine();
  ImGui::SliderFloat("y##location", &y, -2048.f, 2048.f, "%.f");
  entity.mutable_location()->set_x(x);
  entity.mutable_location()->set_y(y);
  if (entity.animation_size() > 0) {
    if (ImGui::TreeNode("Animations")) {
      static char** kAnimationTypeStrings = nullptr;
      if (!kAnimationTypeStrings) {
        CreateProtoEnumStrings(kAnimationTypeStrings, proto::Entity2d_Animation_Type_descriptor());
      }
      for (s32 i = 0; i < entity.animation_size();) {
        proto::Entity2d::Animation* proto_animation = entity.mutable_animation(i);
        snprintf(kTempStr, 128, "x##%s", proto_animation->animation_file().c_str());
        if (ImGui::Button(kTempStr)) {
          entity.mutable_animation()->SwapElements(i, entity.animation_size() - 1);
          entity.mutable_animation()->RemoveLast();
          continue;
        }
        ImGui::SameLine();
        if (ImGui::Selectable(proto_animation->animation_file().c_str())) {
          if (edit_data->anim_sequence) {
            if (!AnimSequence2d::LoadFromProtoFile(
                proto_animation->animation_file().c_str(), edit_data->anim_sequence)) {
              LOG(ERR, "Unable to load proto file %s", proto_animation->animation_file().c_str());
            }
            edit_data->anim_sequence->Start();
          }
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
        ImGui::Combo(kTempStr, (s32*)&type, kAnimationTypeStrings, proto::Entity2d_Animation::Type_MAX + 1);
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
        if (edit_data && edit_data->anim_sequence &&
            edit_data->anim_sequence->file_ == proto_animation->animation_file()) {
          edit_data->anim_sequence->SetAlignment(v2f(alignment_x, alignment_y));
        }
        proto_animation->set_alignment_x(alignment_x);
        proto_animation->set_alignment_y(alignment_y);
        ImGui::Unindent();
        ++i;
      }
      ImGui::TreePop();
    }
  }
  // TODO: Implement entity blueprints or no?
  /*if (entity.has_blueprint()) {
    std::vector<std::string> blueprints = GetEntityBlueprints();;
    static s32 choice = -1;
    if (entity.has_blueprint()) {
      for (s32 i = 0; i < blueprints.size(); ++i) {
        if (entity.blueprint() == blueprints[i]) {
          choice = i;
          break;
        }
      }
    }
    s32 pre_choice = choice;
    if (ImGui::Button("x##blueprint")) {
      choice = -1;
      pre_choice = -1;
      entity.set_blueprint("");
    }
    ImGui::SameLine();
    ImGui::Combo("blueprint", &choice, blueprints[0].c_str(), blueprints.size());
    if (pre_choice != choice) {
      entity.set_blueprint(blueprints[choice]);
    }
  }*/
}
