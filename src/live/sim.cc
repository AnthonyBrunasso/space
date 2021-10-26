#pragma once

using namespace ecs;

namespace live {


template <typename T>
using SimCallbacks = std::vector<std::function<void(const T&)>>;

#define DEFINE_CALLBACK(name, param)                              \
  static SimCallbacks<param> k##name;                             \
  void                                                            \
  Subscribe##name(const std::function<void(const param&)> func) { \
    k##name.push_back(func);                                      \
  }                                                               \
                                                                  \
  void                                                            \
  Dispatch##name(const param& p) {                                \
     for (const auto& event : k##name) {                          \
      event(p);                                                   \
    }                                                             \
  }

struct Sim {
  s32 resources[kResourceTypeCount];
};

static Sim kSim;

u32
SecondsToTicks(r32 seconds)
{
  return (u32)roundf((r32)kGameState.framerate * seconds);
}

#include "live/util.cc"
#include "live/search.cc"
#include "live/grid.cc"
#include "live/sim_create.cc"
#include "live/interaction.cc"
#include "live/order.cc"

void
SimHandleHarvestBoxSelect(const Rectf& selection)
{
  ECS_ITR2(itr, kPhysicsComponent, kHarvestComponent);
  while (itr.Next()) {
    PhysicsComponent* tree = itr.c.physics;
    Rectf trect = tree->rect();
    b8 irect = math::IntersectRect(trect, selection);
    b8 crect = math::IsContainedInRect(trect, selection);
    if (irect || crect) {
      SimCreateHarvestOrder(itr.e);
    }
  }
}

void
SimHandleZoneBoxSelect(const Rectf& selection)
{
  SimCreateZone(selection, 1);
}

void
SimHandleHarvestCompleted(u32 entity_id)
{
  Entity* harvest_entity = FindEntity(entity_id);
  assert(harvest_entity != nullptr);
  HarvestComponent* harvest_component = GetHarvestComponent(harvest_entity);
  assert(harvest_component != nullptr);
  ResourceComponent* resource_component = GetResourceComponent(harvest_entity);
  assert(resource_component != nullptr);
  RemoveHarvestComponent(harvest_entity);
  AssignPickupComponent(harvest_entity);
  // This callback is dispatched in the context of a running order to harvest this thing. Therefore
  // instead of creating a new order we must mutate the existing one or bad things happen.
  OrderComponent* order = GetOrderComponent(harvest_entity);
  order->order_type = kPickup;
  order->acquire_count = 0;
  order->max_acquire_count = 1;
}

void
SimHandleBuildCompleted(u32 entity_id)
{
  Entity* ent = FindEntity(entity_id);
  assert(ent != nullptr);
  BuildComponent* build = GetBuildComponent(ent);
  assert(build != nullptr);
  PhysicsComponent* physics = GetPhysicsComponent(ent);
  assert(physics != nullptr);
  switch (build->structure_type) {
    case kWall:
      SimCreateWall(physics->pos, physics->grid_id);
      break;
    default: printf("Error: SimHandleBuildCompleted - unabled to complete.");
  }
  AssignDeathComponent(entity_id);
}

void
SimHandleBuildLeftClick(v2f pos)
{
  // TODO: Correctly pass grid id.
  SimCreateBuildOrder(kWall, pos, 1, 5.f);
}

void
SimInitialize()
{
  u32 grid_id = GridCreate(v2i(128, 128));

  SimCreateHarvest(kLumber, v2f(400.f, 230.f), grid_id, kSecsToHarvestLumber);
  SimCreateHarvest(kLumber, v2f(472.f, 271.f), grid_id, kSecsToHarvestLumber);
  SimCreateHarvest(kLumber, v2f(428.f, 220.5f), grid_id, kSecsToHarvestLumber);
  SimCreateHarvest(kLumber, v2f(414.f, 215.f), grid_id, kSecsToHarvestLumber);

  for (int x = 0; x < 10; ++x) {
    for (int y = 0; y < 5; ++y) {
      SimCreateHarvest(
          kStone, GridPosFromXY(v2i(30, 30)) + GridPosFromXY(v2i(x, y)),
          grid_id, kSecsToHarvestStone);
    }
  }

  SimCreateCharacter(v2f(160.f, 100.f), grid_id);
  SimCreateCharacter(v2f(80.f, 120.f), grid_id);

  //SimCreateHarvest(kStone, GridPosFromXY(v2i(3, 3)), grid_id, kSecsToHarvestStone);

  SubscribeHarvestBoxSelect(&SimHandleHarvestBoxSelect);
  SubscribeZoneBoxSelect(&SimHandleZoneBoxSelect);
  SubscribeBuildLeftClick(&SimHandleBuildLeftClick);
  SubscribeHarvestCompleted(&SimHandleHarvestCompleted);
  SubscribeBuildCompleted(&SimHandleBuildCompleted);
}

void
SimUpdate()
{
  {
    ECS_ITR2(itr, kCharacterComponent, kPhysicsComponent);
    while (itr.Next()) {
      OrderAcquire(itr.c.character);
      OrderExecute(&itr);
    }
  }

  {
    ECS_ITR2(itr, kPhysicsComponent, kCarryComponent);
    while (itr.Next()) {
      PhysicsComponent* phys = GetPhysicsComponent(itr.e);
      assert(phys != nullptr);
      CarryComponent* carry = GetCarryComponent(itr.e);
      assert(carry != nullptr);
      Entity* carrying_entity = FindEntity(carry->carrier_entity_id);
      assert(carrying_entity != nullptr);
      PhysicsComponent* carrying_physics = GetPhysicsComponent(carrying_entity);
      assert(carrying_physics != nullptr);
      GridSync sync(phys);
      phys->pos = carrying_physics->pos;
    }
  }

  {
    ECS_ITR1(itr, kDeathComponent);
    while (itr.Next()) {
      if (!itr.e) continue;
      // Free the grid of the entity id.
      if (itr.e->Has(kPhysicsComponent)) {
        PhysicsComponent* phys = GetPhysicsComponent(itr.e);
        assert(phys != nullptr);
        GridUnsetEntity(phys);
      }
      if (itr.e->Has(kTagComponent)) {
        TagComponent* tag = GetTagComponent(itr.e);
        assert(tag != nullptr);
        GridUnsetEntity(tag);
      }
      DeleteEntity(itr.e, kComponentCount);
    }
    GetComponents(kDeathComponent)->Clear();
  }

}

}
