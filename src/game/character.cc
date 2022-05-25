#pragma once

class Character;

class CharacterController {
public:
  DECLARE_FRAME_UPDATER(CharacterController)

  static std::unique_ptr<CharacterController> Create(const proto::Entity2d& proto_entity, Character* character);

  void Update();

  Character* character_ = nullptr;
};

class Character : public Entity {
public:
  DECLARE_FRAME_UPDATER(Character)

  static std::unique_ptr<Character> Create(const proto::Entity2d& proto_entity);

  void Update();

  std::unique_ptr<CharacterController> controller_;
};

std::unique_ptr<Character> Character::Create(const proto::Entity2d& proto_entity) {
  std::unique_ptr<Character> character(new Character);
  if (proto_entity.is_player()) {
    character->controller_ = CharacterController::Create(proto_entity, character.get());
  }
  return character;
}

void Character::Update() {
}

std::unique_ptr<CharacterController> CharacterController::Create(
    const proto::Entity2d& proto_entity, Character* character) {
  std::unique_ptr<CharacterController> controller(new CharacterController);
  controller->character_ = character;
  return controller;
}

void CharacterController::Update() {
  if (Input::Get().IsKeyDown(KEY_W)) {
    character_->pos_.y += 1.f;
  }
  if (Input::Get().IsKeyDown(KEY_S)) {
    character_->pos_.y -= 1.f;
  }
  if (Input::Get().IsKeyDown(KEY_D)) {
    character_->pos_.x += 1.f;
  }
  if (Input::Get().IsKeyDown(KEY_A)) {
    character_->pos_.x -= 1.f;
  }
}
