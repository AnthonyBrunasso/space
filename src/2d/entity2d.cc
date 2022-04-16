#pragma once

#include "entity.pb.h"

bool EntityLoadIdleAnimation(const proto::Entity2d& entity, AnimSequence2d* anim_sequence) {
  bool loaded = false;
  for (const proto::Entity2d::Animation& anim : entity.animation()) {
    if (anim.type() == proto::Entity2d_Animation::kIdle) {
      loaded = AnimSequence2d::LoadFromProtoFile(anim.animation_file().c_str(), anim_sequence);
      break;
    }
  }
  return loaded;
}
