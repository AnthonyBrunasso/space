#pragma once

class Entity {
public:
  virtual ~Entity() = default;
  u32 id_;
  v2f pos_;
  proto::Entity2d::Type type_;
};

std::vector<std::function<void()>> kUpdaters;

#define DECLARE_FRAME_UPDATER(type)          \
public:                                      \
  static std::vector<type*>& Array() {       \
    static bool kAddToUpdater = true;        \
    if (kAddToUpdater) {                     \
      kUpdaters.push_back(&type::UpdateAll); \
      kAddToUpdater = false;                 \
    }                                        \
    static std::vector<type*> k##typeList;   \
    return k##typeList;                      \
  }                                          \
private:                                     \
  void AddUpdater(type* c) {                 \
    Array().push_back(c);                    \
  }                                          \
                                             \
  void RemoveUpdater(type* c) {              \
    Array().erase(std::remove(               \
        Array().begin(),                     \
        Array().end(),                       \
        c), Array().end());                  \
  }                                          \
                                             \
  type() {                                   \
    AddUpdater(this);                        \
  }                                          \
                                             \
  type(const type& rhs) = delete;            \
                                             \
public:                                      \
  ~type() {                                  \
    RemoveUpdater(this);                     \
  }                                          \
                                             \
  static void UpdateAll() {                  \
    for (type* t : Array())                  \
      t->Update();                           \
  }



#include "character.cc"

typedef std::unique_ptr<Entity> EntityPtr;
static std::unordered_map<u32, EntityPtr> kEntities;
static u32 kEntityInvalid = 0;
static u32 kEntityAutoIncrementId = 1;

void EntityCreateFromProto(const proto::Entity2d& proto_entity) {
  std::unique_ptr<Entity> entity;
  switch (proto_entity.type()) {
    case proto::Entity2d::kCharacter: {
      entity = Character::Create(proto_entity);
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
  kEntities[id] = std::move(entity);
}

void EntityDestroy(u32 entity_id) {
  Entity* entity = FindOrNull(kEntities, entity_id);
  if (!entity) return;
}

void EntityRunUpdates() {
  for (const auto& func : kUpdaters) func();
}
