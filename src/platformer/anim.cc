#pragma once

class Anim {
public:
  void Initialize(const proto::Entity2d& proto_entity);

  void Update();
  void Signal(proto::Entity2d::Animation::Type anim_type);
  const AnimFrame2d* CurrentFrame();

  std::unordered_map<u32, AnimSequence2d> sequence_map_;
  AnimSequence2d* current_sequence_ = nullptr;
};

void Anim::Initialize(const proto::Entity2d& proto_entity) {
  if (proto_entity.animation_size() == 0)
    return nullptr;
  for (const proto::Entity2d::Animation& proto_anim : proto_entity.animation()) {
    AnimSequence2d& anim_sequence = sequence_map_[(u32)proto_anim.type()];
    if (!AnimSequence2d::LoadFromProtoFile(proto_anim.animation_file(), &anim_sequence)) {
      LOG(ERR, "Error loading animation %s", proto_anim.animation_file().c_str());
      continue;
    }
    anim_sequence.alignment_ = v2f(proto_anim.alignment_x(), proto_anim.alignment_y());
  }
  current_sequence_ = FindOrNull(sequence_map_, (u32)proto::Entity2d::Animation::kIdle);
}

void Anim::Update() {
  //LOG(INFO, "Anim::Update");

  // TODO: Set current sequence if it doesn't have one.
  if (!current_sequence_)
    return;

  if (!current_sequence_->IsActive())
    current_sequence_->Start();

  current_sequence_->Update();
}

void Anim::Signal(proto::Entity2d::Animation::Type anim_type) {
  AnimSequence2d* transition = FindOrNull(sequence_map_, (u32)anim_type);
  if (!transition) {
    LOG(ERR, "Couldn't find valid anim transition for type %u", (u32)anim_type);
    return;
  }
  if (transition != current_sequence_) {
    current_sequence_ = transition;
    current_sequence_->Start();
  }
}

const AnimFrame2d* Anim::CurrentFrame() {
  if (!current_sequence_)
    return nullptr;

  return &current_sequence_->CurrentFrame();
}
