#pragma once


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

#include "live/order.cc"
#include "live/interaction.cc"

void
SimCreateTree(v2f pos)
{
  ecs::Entity* tree = ecs::UseEntity();
  ecs::PhysicsComponent* comp = ecs::AssignPhysicsComponent(tree);
  comp->pos = pos;
  comp->bounds = v2f(live::kTreeWidth, live::kTreeHeight);
  ecs::AssignTreeComponent(tree);
}

void SimCreateCharacter(v2f pos)
{
  ecs::Entity* character = ecs::UseEntity();
  ecs::PhysicsComponent* comp = ecs::AssignPhysicsComponent(character);
  comp->pos = pos;
  comp->bounds = v2f(live::kCharacterWidth, live::kCharacterHeight);
  ecs::AssignCharacterComponent(character);
}

void
SimHandleBoxSelect(const Rectf& selection)
{
  ECS_ITR2(itr, ecs::kPhysicsComponent, ecs::kTreeComponent);
  while (itr.Next()) {
    ecs::PhysicsComponent* tree = itr.c.physics;
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
  assert(ecs::FindEntity(entity_id) != nullptr);
  ecs::AssignDeathComponent(entity_id);
}

void
SimInitialize()
{
  SimCreateTree(v2f(0.f, 0.f));
  SimCreateTree(v2f(15.f, 8.f));
  SimCreateTree(v2f(-8.f, -12.5f));
  SimCreateTree(v2f(-4.f, 20.f));

  SimCreateCharacter(v2f(-80.f, 100.f));
  SimCreateCharacter(v2f(80.f, 120.f));

  SubscribeBoxSelect(&SimHandleBoxSelect);
  SubscribeHarvestCompleted(&SimHandleHarvestCompleted);
}

void
SimUpdate()
{
  if (kInteraction.left_mouse_down) {
    Rectf srect = math::OrientToAabb(kInteraction.selection_rect());
    ECS_ITR2(itr, ecs::kPhysicsComponent, ecs::kTreeComponent);
    while (itr.Next()) {
      ecs::PhysicsComponent* tree = itr.c.physics;
      Rectf trect = tree->rect();
      b8 irect = math::IntersectRect(trect, srect);
      b8 crect = math::IsContainedInRect(trect, srect);
      if (irect || crect) {
        Rectf render_rect = trect;
        render_rect.x -= 1.f;
        render_rect.y -= 1.f;
        render_rect.width += 2.f;
        render_rect.height += 2.f;
        rgg::DebugPushRect(render_rect, v4f(1.f, 1.f, 1.f, 1.f));
      }
    }
    rgg::DebugPushRect(srect, v4f(0.f, 1.f, 0.f, 0.2f));
  }

  {
    ECS_ITR2(itr, ecs::kCharacterComponent, ecs::kPhysicsComponent);
    while (itr.Next()) {
      OrderAcquire(itr.c.character);
      OrderExecute(&itr);
    }
  }

  {
    ECS_ITR1(itr, ecs::kDeathComponent);
    while (itr.Next()) {
      // Just in case an entity got multiple death components.
      if (!itr.e) continue;
      ecs::DeleteEntity(itr.e, ecs::kComponentCount);
    }
    ecs::GetComponents(ecs::kDeathComponent)->Clear();
  }

}

}
