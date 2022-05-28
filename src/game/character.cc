#pragma once

class Controller;
class Anim;

class Character : public Entity {
public:
  DECLARE_FRAME_UPDATER(Character)

  static std::unique_ptr<Character> Create(const proto::Entity2d& proto_entity);

  void Update();

  Controller* controller() { return controller_.get(); }
  Anim*       anim()       { return anim_.get();       }

  std::unique_ptr<Controller> controller_;
  std::unique_ptr<Anim> anim_;
};

#include "anim.cc"
#include "controller.cc"

std::unique_ptr<Character> Character::Create(const proto::Entity2d& proto_entity) {
  std::unique_ptr<Character> character(new Character);
  character->controller_ = proto_entity.is_player() ?
      PlayerController::Create(proto_entity, character.get()) : nullptr;
  character->anim_ = Anim::Create(proto_entity);
  return character;
}

void Character::Update() {
}
