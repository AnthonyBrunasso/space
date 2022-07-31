#pragma once

class Controller;
class Anim;

#include "anim.cc"

class Character : public Entity {
public:
  static std::unique_ptr<Character> Create(const proto::Entity2d& proto_entity);

  void OnUpdate() override;

  std::unique_ptr<Controller> controller_;

  Anim anim_;
};

#include "controller.cc"

std::unique_ptr<Character> Character::Create(const proto::Entity2d& proto_entity) {
  std::unique_ptr<Character> character(new Character);
  character->controller_ = proto_entity.is_player() ?
      PlayerController::Create(proto_entity, character.get()) : nullptr;
  character->anim_.Initialize(proto_entity);
  character->has_update_ = true;
  return character;
}

void Character::OnUpdate() {
  controller_->OnUpdate();
  //anim_.Update();
}
