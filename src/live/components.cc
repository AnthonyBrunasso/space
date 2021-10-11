namespace ecs {

enum TypeId : u64 {
  kPhysicsComponent = 0,
  kHarvestComponent = 1,
  kCharacterComponent = 2,
  kDeathComponent = 3,
  kBuildComponent = 4,
  kStructureComponent = 5,
  kOrderComponent = 6,
  kComponentCount = 7,
};

const char*
TypeName(TypeId type_id)
{
  switch (type_id) {
    case kPhysicsComponent: return "Physics";
    case kHarvestComponent: return "Harvest";
    case kCharacterComponent: return "Character";
    case kDeathComponent: return "Death";
    case kBuildComponent: return "Build";
    case kStructureComponent: return "Structure";
    case kOrderComponent: return "Order";
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
  // All things that appear in the world only exist in the context of a specific
  // grid. This value represents that index in grid.cc:kGrids.
  u32 grid_id;
  Rectf rect() const { return Rectf(pos, bounds); }
};

enum ResourceType {
  kLumber = 0,
  kStone = 1,
  kResourceTypeCount = 2,
};

const char*
ResourceName(ResourceType type)
{
  switch (type) {
    case kLumber: return "Lumber";
    case kStone: return "Stone";
    case kResourceTypeCount: return "Unknown";
  }
  return "Unknown";
}

struct HarvestComponent {
  u32 entity_id;
  ResourceType resource_type;
  // ticks_to_harvest.
  u32 tth;
};

struct CharacterComponent {
  u32 entity_id;
  u32 order_id; // TEMP
};

// If set on an entity will remove it at the end of the current frame.
struct DeathComponent {
  u32 entity_id = 0;
};

enum StructureType {
  kWall = 0,
  kStructureTypeCount = 1,
};

const char*
StructureName(StructureType type)
{
  switch (type) {
    case kWall: return "Wall";
    default: return "Unknown";
  }
  return "Unknown";
}

struct BuildComponent {
  u32 entity_id;
  StructureType structure_type;
  ResourceType required_resource_type;
  u32 resource_count = 0;
  // ticks_to_build
  u32 ttb;
};

struct StructureComponent {
  u32 entity_id;
  StructureType structure_type;
};

enum OrderType {
  kMove = 0,
  kHarvest = 1,
  kBuild = 2,
  kOrderTypeCount = 3,
};

struct OrderComponent {
  u32 entity_id;
  OrderType order_type;
  // Number of units that have acquired this order.
  u32 acquire_count = 0;
  u32 max_acquire_count = 0;
};

}

namespace ecs {

struct Components {
  PhysicsComponent* physics = nullptr;
  HarvestComponent* harvest = nullptr;
  CharacterComponent* character = nullptr;
  DeathComponent* death = nullptr;
  BuildComponent* build = nullptr;
  StructureComponent* structure = nullptr;
  OrderComponent* order = nullptr;
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
      static ecs::ComponentStorage f(ENTITY_COUNT, sizeof(PhysicsComponent), kPhysicsComponent);
      return &f;
    } break;
    case kHarvestComponent: {
      static ecs::ComponentStorage f(128, sizeof(HarvestComponent), kHarvestComponent);
      return &f;
    } break;
    case kCharacterComponent: {
      static ecs::ComponentStorage f(128, sizeof(CharacterComponent), kCharacterComponent);
      return &f;
    } break;
    case kDeathComponent: {
      static ecs::ComponentStorage f(64, sizeof(DeathComponent), kDeathComponent);
      return &f;
    } break;
    case kBuildComponent: {
      static ecs::ComponentStorage f(64, sizeof(BuildComponent), kBuildComponent);
      return &f;
    } break;
    case kStructureComponent: {
      static ecs::ComponentStorage f(64, sizeof(StructureComponent), kStructureComponent);
      return &f;
    } break;
    case kOrderComponent: {
      static ecs::ComponentStorage f(64, sizeof(OrderComponent), kOrderComponent);
      return &f;
    } break;
    default: {
      assert(!"Unknown component type");
    } break;
  }
  return nullptr;
}

DECLARE_COMPONENT(PhysicsComponent, kPhysicsComponent);
DECLARE_COMPONENT(HarvestComponent, kHarvestComponent);
DECLARE_COMPONENT(CharacterComponent, kCharacterComponent);
DECLARE_COMPONENT(DeathComponent, kDeathComponent);
DECLARE_COMPONENT(BuildComponent, kBuildComponent);
DECLARE_COMPONENT(StructureComponent, kStructureComponent);
DECLARE_COMPONENT(OrderComponent, kOrderComponent);

}
