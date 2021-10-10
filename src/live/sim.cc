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

#include "live/order.cc"
#include "live/interaction.cc"

u32
SecondsToTicks(r32 seconds)
{
  return (u32)roundf((r32)kGameState.framerate * seconds);
}

void
SimCreateHarvest(v2f pos, ResourceType resource_type, r32 seconds_to_harvest)
{
  Entity* entity = UseEntity();
  PhysicsComponent* comp = AssignPhysicsComponent(entity);
  comp->pos = pos;
  switch (resource_type) {
    case kLumber:
      comp->bounds = v2f(live::kLumberWidth, live::kLumberHeight);
      break;
    case kStone:
      comp->bounds = v2f(live::kStoneWidth, live::kStoneHeight);
      break;
  }
  HarvestComponent* harvest = AssignHarvestComponent(entity);
  harvest->resource_type = resource_type;
  harvest->tth = SecondsToTicks(seconds_to_harvest);
}

void
SimCreateBuild(v2f pos, StructureType structure_type, r32 seconds_to_build)
{
  Entity* entity = UseEntity();
  PhysicsComponent* comp = AssignPhysicsComponent(entity);
  comp->pos = pos - v2f(kWallWidth  / 2.f, kWallHeight / 2.f);
  switch (structure_type) {
    case kWall:
      comp->bounds = v2f(live::kWallWidth, live::kWallHeight);
      break;
  }
  BuildComponent* build = AssignBuildComponent(entity);
  build->structure_type = structure_type;
  build->ttb = SecondsToTicks(seconds_to_build);
  build->required_resource_type = kLumber;
  build->resource_count = 1;
  OrderCreateBuild(entity);
}

void
SimCreateCharacter(v2f pos)
{
  Entity* character = UseEntity();
  PhysicsComponent* comp = AssignPhysicsComponent(character);
  comp->pos = pos;
  comp->bounds = v2f(live::kCharacterWidth, live::kCharacterHeight);
  AssignCharacterComponent(character);
}

void
SimCreateWall(v2f pos)
{
  Entity* wall = UseEntity();
  PhysicsComponent* comp = AssignPhysicsComponent(wall);
  comp->pos = pos;
  comp->bounds = v2f(live::kWallWidth, live::kWallHeight);
  StructureComponent* structure = AssignStructureComponent(wall);
  structure->structure_type = kWall; 
}

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
      OrderCreateHarvest(itr.e);
    }
  }
}

void
SimHandleHarvestCompleted(u32 entity_id)
{
  Entity* harvest_entity = FindEntity(entity_id);
  assert(harvest_entity != nullptr);
  HarvestComponent* harvest_component = GetHarvestComponent(harvest_entity);
  assert(harvest_component != nullptr);
  ++kSim.resources[harvest_component->resource_type];
  AssignDeathComponent(entity_id);
}

void
SimHandleBuildCompleted(u32 entity_id)
{
  //printf("Build completed\n");
  Entity* ent = FindEntity(entity_id);
  assert(ent != nullptr);
  BuildComponent* build = GetBuildComponent(ent);
  assert(build != nullptr);
  PhysicsComponent* physics = GetPhysicsComponent(ent);
  assert(physics != nullptr);
  switch (build->structure_type) {
    case kWall:
      SimCreateWall(physics->pos);
      break;
    default: printf("Error: SimHandleBuildCompleted - unabled to complete.");
  }
  AssignDeathComponent(entity_id);
}

void
SimHandleBuildLeftClick(v2f pos)
{
  //printf("Build left click %.2f %.2f\n", pos.x, pos.y);
  SimCreateBuild(pos, kWall, 5.f);
}

void
SimInitialize()
{
  SimCreateHarvest(v2f(0.f, 0.f), kLumber, kSecsToHarvestLumber);
  SimCreateHarvest(v2f(15.f, 8.f), kLumber, kSecsToHarvestLumber);
  SimCreateHarvest(v2f(-8.f, -12.5f), kLumber, kSecsToHarvestLumber);
  SimCreateHarvest(v2f(-4.f, 20.f), kLumber, kSecsToHarvestLumber);

  for (int x = 0; x < 10; ++x) {
    for (int y = 0; y < 5; ++y) {
      SimCreateHarvest(v2f(400.f + x * (kStoneWidth + 5.f), 400.f - y * (kStoneHeight + 5.f)), kStone, kSecsToHarvestStone);
    }
  }

  SimCreateCharacter(v2f(-80.f, 100.f));
  SimCreateCharacter(v2f(80.f, 120.f));

  SubscribeHarvestBoxSelect(&SimHandleHarvestBoxSelect);
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
    ECS_ITR1(itr, kDeathComponent);
    while (itr.Next()) {
      // Just in case an entity got multiple death components.
      if (!itr.e) continue;
      DeleteEntity(itr.e, kComponentCount);
    }
    GetComponents(kDeathComponent)->Clear();
  }

}

}
