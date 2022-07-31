#pragma once


#include "character.cc"

typedef std::unique_ptr<Entity> EntityPtr;
static std::unordered_map<u32, EntityPtr> kEntities;
static u32 kEntityInvalid = 0;
static u32 kEntityAutoIncrementId = 1;
static std::vector<Character*> kCharacters;

void EntityCreateFromProto(const proto::Entity2d& proto_entity) {
  std::unique_ptr<Entity> entity;
  switch (proto_entity.type()) {
    case proto::Entity2d::kCharacter: {
      entity = Character::Create(proto_entity);
      kCharacters.push_back((Character*)entity.get());
    } break;
    case proto::Entity2d::kUndefined:
    default:
      break;
  }
  u32 id = kEntityAutoIncrementId++;
  entity->id_ = id;
  entity->type_ = proto_entity.type();
  entity->pos_.x = proto_entity.location().x();
  entity->pos_.y = proto_entity.location().y();
  PAddEntity(entity->id_);
  kEntities[id] = std::move(entity);
}

Entity* EntityGet(u32 entity_id) {
  return FindOrNull(kEntities, entity_id);
}

void EntityDestroy(u32 entity_id) {
  Entity* entity = FindOrNull(kEntities, entity_id);
  if (!entity) return;
  switch(entity->type_) {
    case proto::Entity2d::kCharacter: {
      kCharacters.erase(std::remove(kCharacters.begin(), kCharacters.end(), entity), kCharacters.end());
    } break;
    case proto::Entity2d::kUndefined:
    default:
      break;
  }
}

void EntityUpdate() {
  for (std::pair<const u32, EntityPtr>& pair : kEntities) {
    EntityPtr& entity = pair.second;
    if (entity->has_update_) {
      entity->OnUpdate();
    }
  }
}
