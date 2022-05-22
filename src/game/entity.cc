#pragma once

class Entity {
public:
  u32 id_;
  v2f pos_;
  proto::Entity2d::Type type_;
};

#define DECLARE_FRAME_UPDATER(type)          \
public:                                      \
  static std::vector<type*>& UpdateQueue() { \
    static std::vector<type*> k##typeList;   \
    return k##typeList;                      \
  }                                          \
private:                                     \
  void AddUpdater(type* c) {                 \
    UpdateQueue().push_back(c);              \
  }                                          \
                                             \
  void RemoveUpdater(type* c) {              \
    UpdateQueue().erase(std::remove(         \
        UpdateQueue().begin(),               \
        UpdateQueue().end(),                 \
        c), UpdateQueue().end());            \
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
    for (type* t : UpdateQueue())            \
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
  kEntities[id] = std::move(entity);
}

void EntityDestroy(u32 entity_id) {
  Entity* entity = FindOrNull(kEntities, entity_id);
  if (!entity) return;
}

void EntityRunUpdates() {
  // TODO: Can I somehow automatically generate the call to all of these???
  Character::UpdateAll();
}
