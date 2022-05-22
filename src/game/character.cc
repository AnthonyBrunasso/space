#pragma once

class Character : public Entity {
public:
  DECLARE_FRAME_UPDATER(Character)

  static std::unique_ptr<Character> Create(const proto::Entity2d& proto_entity);

  void Update();
};

std::unique_ptr<Character> Character::Create(const proto::Entity2d& proto_entity) {
  std::unique_ptr<Character> character(new Character);
  return character;
}

void Character::Update() {
  LOG(INFO, "Runing update for entity %u", id_);
}

