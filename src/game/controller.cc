#pragma once

class Controller {
public:
  Controller() = default;
  virtual ~Controller() = default;

  virtual void OnUpdate() = 0;
};

class PlayerController : public Controller {
public:
  static std::unique_ptr<PlayerController> Create(const proto::Entity2d& proto_entity, Character* character);

  void OnUpdate() override;

  Character* character_ = nullptr;
};

std::unique_ptr<PlayerController> PlayerController::Create(
    const proto::Entity2d& proto_entity, Character* character) {
  std::unique_ptr<PlayerController> controller(new PlayerController);
  controller->character_ = character;
  return controller;
}

void PlayerController::OnUpdate() {
  b8 signal_idle = true;
  if (Input::Get().IsKeyDown(KEY_W)) {
    character_->pos_.y += 1.f;
    character_->anim_.Signal(proto::Entity2d::Animation::kJump);
    signal_idle = false;
  }

  if (Input::Get().IsKeyDown(KEY_S)) {
    character_->pos_.y -= 1.f;
  }

  if (Input::Get().IsKeyDown(KEY_D)) {
    character_->pos_.x += 1.f;
    if (signal_idle) character_->anim_.Signal(proto::Entity2d::Animation::kMove);
    signal_idle = false;
  }

  if (Input::Get().IsKeyDown(KEY_A)) {
    character_->pos_.x -= 1.f;
    if (signal_idle) character_->anim_.Signal(proto::Entity2d::Animation::kMove);
    signal_idle = false;
  }

  if (signal_idle)
    character_->anim_.Signal(proto::Entity2d::Animation::kIdle);
}
