namespace ecs {

enum TypeId : u64 {
  kPhysicsComponent = 0,
  kTreeComponent = 1,
  kCharacterComponent = 2,
  kDeathComponent = 3,
  kComponentCount = 4,
};

const char*
TypeName(TypeId type_id)
{
  switch (type_id) {
    case kPhysicsComponent: return "Physics";
    case kTreeComponent: return "Tree";
    case kCharacterComponent: return "Character";
    case kDeathComponent: return "Death";
    default: return "Unknown";
  }
  return "Unknown";
}

const char*
TypeName(u64 type_id)
{
  return TypeName((TypeId)type_id);
}

struct PhysicsComponent {
  u32 entity_id;
  v2f pos;
  v2f bounds;
  Rectf rect() const { return Rectf(pos, bounds); }
};

struct TreeComponent {
  u32 entity_id;
};

struct CharacterComponent {
  u32 entity_id;
  u32 order_id; // TEMP
};

// If set on an entity will remove it at the end of the current frame.
struct DeathComponent {
  u32 entity_id = 0;
};

}

namespace ecs {

struct Components {
  PhysicsComponent* physics = nullptr;
  TreeComponent* tree = nullptr;
  CharacterComponent* character = nullptr;
  DeathComponent* death = nullptr;
};

}

#define ENTITY_COUNT 2048
#include "ecs/ecs.cc"

namespace ecs {

ComponentStorage*
GetComponents(u64 tid)
{
  switch (tid) {
    case kPhysicsComponent: {
      static ecs::ComponentStorage f(ENTITY_COUNT, sizeof(PhysicsComponent));
      return &f;
    } break;
    case kTreeComponent: {
      static ecs::ComponentStorage f(128, sizeof(TreeComponent));
      return &f;
    } break;
    case kCharacterComponent: {
      static ecs::ComponentStorage f(128, sizeof(CharacterComponent));
      return &f;
    } break;
    case kDeathComponent: {
      static ecs::ComponentStorage f(64, sizeof(DeathComponent));
      return &f;
    } break;
    default: {
      assert(!"Unknown component type");
    } break;
  }
  return nullptr;
}

DECLARE_COMPONENT(PhysicsComponent, kPhysicsComponent);
DECLARE_COMPONENT(TreeComponent, kTreeComponent);
DECLARE_COMPONENT(CharacterComponent, kCharacterComponent);
DECLARE_COMPONENT(DeathComponent, kDeathComponent);

}
