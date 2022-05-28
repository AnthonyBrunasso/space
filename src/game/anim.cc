#pragma once

class Anim {
public:
  DECLARE_FRAME_UPDATER(Anim)

  static std::unique_ptr<Anim> Create(const proto::Entity2d& proto_entity);

  void Update();
  const AnimFrame2d* CurrentFrame();

  std::unordered_map<u32, AnimSequence2d> sequence_map_;
  AnimSequence2d* current_sequence_ = nullptr;
};


std::unique_ptr<Anim> Anim::Create(const proto::Entity2d& proto_entity) {
  if (proto_entity.animation_size() == 0)
    return nullptr;
  std::unique_ptr<Anim> anim(new Anim);
  for (const proto::Entity2d::Animation& proto_anim : proto_entity.animation()) {
    AnimSequence2d& anim_sequence = anim->sequence_map_[(u32)proto_anim.type()];
    if (!AnimSequence2d::LoadFromProtoFile(proto_anim.animation_file(), &anim_sequence)) {
      LOG(ERR, "Error loading animation %s", proto_anim.animation_file().c_str());
      continue;
    }
    anim_sequence.alignment_ = v2f(proto_anim.alignment_x(), proto_anim.alignment_y());
  }
  anim->current_sequence_ = FindOrNull(anim->sequence_map_, (u32)proto::Entity2d::Animation::kIdle);
  return anim;
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

const AnimFrame2d* Anim::CurrentFrame() {
  if (!current_sequence_)
    return nullptr;

  return &current_sequence_->CurrentFrame();
}
