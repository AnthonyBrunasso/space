namespace ecs {

enum TypeId : u64 {
  kPhysicsComponent = 0,
  kResourceComponent = 1,
  kHarvestComponent = 2,
  kCharacterComponent = 3,
  kDeathComponent = 4,
  kBuildComponent = 5,
  kStructureComponent = 6,
  kOrderComponent = 7,
  kPickupComponent = 8,
  kZoneComponent = 9,
  kCarryComponent = 10,
  kTagComponent = 11,
  kComponentCount = 12,
};

const char*
TypeName(TypeId type_id)
{
  switch (type_id) {
    case kPhysicsComponent: return "Physics";
    case kResourceComponent: return "Resource";
    case kHarvestComponent: return "Harvest";
    case kCharacterComponent: return "Character";
    case kDeathComponent: return "Death";
    case kBuildComponent: return "Build";
    case kStructureComponent: return "Structure";
    case kOrderComponent: return "Order";
    case kPickupComponent: return "Pickup";
    case kZoneComponent: return "Zone";
    case kCarryComponent: return "Carry";
    case kTagComponent: return "Tag";
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

struct ResourceComponent {
  u32 entity_id;
  ResourceType resource_type;
};

struct HarvestComponent {
  u32 entity_id;
  // ticks_to_harvest.
  u32 tth;
};

struct CharacterComponent {
  u32 entity_id;
  u32 order_id;
  u32 carrying_id;
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
  u32 pickup_orders_issued = 0;
};

struct StructureComponent {
  u32 entity_id;
  StructureType structure_type;
};

enum OrderType {
  kMove = 0,
  kHarvest = 1,
  kBuild = 2,
  kPickup = 3,
  kCarryTo = 4,
  kOrderTypeCount = 5,
};

struct HarvestData {};
struct BuildData {};
struct PickupData {
  enum Destination {
    kFindZone = 0,
    kBuild = 1,
  };
  Destination destination;
  u32 build_entity_id = 0;
};
struct CarryToData {
  u32 target_entity_id;
};

struct OrderComponent {
  u32 entity_id;
  OrderType order_type;
  // Number of units that have acquired this order.
  u32 acquire_count = 0;
  u32 max_acquire_count = 0;
  u32 target_entity_id = 0;
  union {
    HarvestData harvest_data;
    BuildData build_data;
    PickupData pickup_data;
    CarryToData carry_to_data;
  };
};

struct PickupComponent {
  u32 entity_id;
};

struct ZoneComponent {
  u32 entity_id;
  // Specifies the types of resources allowed to be placed in this zone.
  u32 resource_mask;
  // Zones start and end at grid locations.
  v2i zone_start;
  v2i zone_end;
};

struct CarryComponent {
  u32 entity_id;
  u32 carrier_entity_id;
};

// Not sure how I feel about this - but just want to use this as a component to indicate something
// is interesting about this entity. Right now the case is I want to temporarily create an entity
// with this component in a zone for a character to carry a resource to.
struct TagComponent {
  u32 entity_id;
  v2i grid_pos;
  u32 grid_id;
};

}

namespace ecs {

struct Components {
  PhysicsComponent* physics = nullptr;
  ResourceComponent* resource = nullptr;
  HarvestComponent* harvest = nullptr;
  CharacterComponent* character = nullptr;
  DeathComponent* death = nullptr;
  BuildComponent* build = nullptr;
  StructureComponent* structure = nullptr;
  OrderComponent* order = nullptr;
  PickupComponent* pickup = nullptr;
  ZoneComponent* zone = nullptr;
  CarryComponent* carry = nullptr;
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
    case kResourceComponent: {
      static ecs::ComponentStorage f(256, sizeof(ResourceComponent), kResourceComponent);
      return &f;
    } break;
    case kHarvestComponent: {
      static ecs::ComponentStorage f(256, sizeof(HarvestComponent), kHarvestComponent);
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
    case kPickupComponent: {
      static ecs::ComponentStorage f(64, sizeof(PickupComponent), kPickupComponent);
      return &f;
    } break;
    case kZoneComponent: {
      static ecs::ComponentStorage f(64, sizeof(ZoneComponent), kZoneComponent);
      return &f;
    } break;
    case kCarryComponent: {
      static ecs::ComponentStorage f(64, sizeof(CarryComponent), kCarryComponent);
      return &f;
    } break;
    case kTagComponent: {
      static ecs::ComponentStorage f(64, sizeof(TagComponent), kTagComponent);
      return &f;
    } break;
    default: {
      assert(!"Unknown component type");
    } break;
  }
  return nullptr;
}

DECLARE_COMPONENT(PhysicsComponent, kPhysicsComponent);
DECLARE_COMPONENT(ResourceComponent, kResourceComponent);
DECLARE_COMPONENT(HarvestComponent, kHarvestComponent);
DECLARE_COMPONENT(CharacterComponent, kCharacterComponent);
DECLARE_COMPONENT(DeathComponent, kDeathComponent);
DECLARE_COMPONENT(BuildComponent, kBuildComponent);
DECLARE_COMPONENT(StructureComponent, kStructureComponent);
DECLARE_COMPONENT(OrderComponent, kOrderComponent);
DECLARE_COMPONENT(PickupComponent, kPickupComponent);
DECLARE_COMPONENT(ZoneComponent, kZoneComponent);
DECLARE_COMPONENT(CarryComponent, kCarryComponent);
DECLARE_COMPONENT(TagComponent, kTagComponent);

}
