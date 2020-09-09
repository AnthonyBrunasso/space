#pragma once

namespace mood
{

enum AdventurerAnimState {
  kAdventurerAnimIdle      = 0,
  kAdventurerAnimWalk      = 1,
  kAdventurerAnimJump      = 2,
  kAdventurerAnimNumStates = 3,
};


using animation::FSM;

DECLARE_HASH_ARRAY(FSM, 256);

u32
AnimUseAdventurerFSM(u32 entity_id)
{
  FSM* fsm = UseFSM();

  fsm->Initialize(kAdventurerAnimNumStates, kAdventurerAnimIdle);
  fsm->entity_id = entity_id;

  fsm->Node(kAdventurerAnimIdle)
      .Frame(50.f * 0.f, 0.f, 50.f, 37.f, 25)
      .Frame(50.f * 1.f, 0.f, 50.f, 37.f, 25)
      .Frame(50.f * 2.f, 0.f, 50.f, 37.f, 25)
      .Frame(50.f * 3.f, 0.f, 50.f, 37.f, 25)
      .Transition(
          kAdventurerAnimWalk,
          [](u32 entity_id) {
            ecs::Entity* entity = ecs::FindEntity(entity_id);
            PhysicsComponent* phys = ecs::GetPhysicsComponent(entity);
            if (!phys) return false;
            physics::Particle2d* p =
                physics::FindParticle2d(phys->particle_id);
            if (!p) return false;
            return fabs(p->velocity.x) > 0.f;
          })
      .Transition(
          kAdventurerAnimJump,
          [](u32 entity_id) {
            ecs::Entity* entity = ecs::FindEntity(entity_id);
            PhysicsComponent* phys = ecs::GetPhysicsComponent(entity);
            if (!phys) return false;
            physics::Particle2d* p =
                physics::FindParticle2d(phys->particle_id);
            if (!p) return false;
            return !p->on_ground;
          });

  fsm->Node(kAdventurerAnimWalk)
      .Frame(50.f * 1.f, 37.f, 50.f, 37.f, 15)
      .Frame(50.f * 2.f, 37.f, 50.f, 37.f, 15)
      .Frame(50.f * 3.f, 37.f, 50.f, 37.f, 15)
      .Frame(50.f * 4.f, 37.f, 50.f, 37.f, 15)
      .Frame(50.f * 5.f, 37.f, 50.f, 37.f, 15)
      .Frame(50.f * 6.f, 37.f, 50.f, 37.f, 15)
      .Transition(
          kAdventurerAnimIdle,
          [](u32 entity_id) {
            ecs::Entity* entity = ecs::FindEntity(entity_id);
            PhysicsComponent* phys = ecs::GetPhysicsComponent(entity);
            if (!phys) return false;
            physics::Particle2d* p =
                physics::FindParticle2d(phys->particle_id);
            if (!p) return false;
            return p->velocity.x == 0.f;
          })
      .Transition(
          kAdventurerAnimJump,
          [](u32 entity_id) {
            ecs::Entity* entity = ecs::FindEntity(entity_id);
            PhysicsComponent* phys = ecs::GetPhysicsComponent(entity);
            if (!phys) return false;
            physics::Particle2d* p =
                physics::FindParticle2d(phys->particle_id);
            if (!p) return false;
            return !p->on_ground;
          });

  fsm->Node(kAdventurerAnimJump)
      .Frame(50.f * 0.f, 37.f * 2.f, 50.f, 37.f, 2)
      .Frame(50.f * 1.f, 37.f * 2.f, 50.f, 37.f, 2)
      .Frame(50.f * 2.f, 37.f * 2.f, 50.f, 37.f, 10)
      .Frame(50.f * 3.f, 37.f * 2.f, 50.f, 37.f, 10)
      .Frame(50.f * 4.f, 37.f * 2.f, 50.f, 37.f, 10)
      .Frame(50.f * 5.f, 37.f * 2.f, 50.f, 37.f, 10)
      .Frame(50.f * 6.f, 37.f * 2.f, 50.f, 37.f, 10)
      .Flag(animation::kFSMNodeNoCycle)
      .Transition(
          kAdventurerAnimIdle,
          [](u32 entity_id) {
            ecs::Entity* entity = ecs::FindEntity(entity_id);
            PhysicsComponent* phys = ecs::GetPhysicsComponent(entity);
            if (!phys) return false;
            physics::Particle2d* p =
                physics::FindParticle2d(phys->particle_id);
            if (!p) return false;
            return p->on_ground != 0;
          });

  return fsm->id;
}

void
AnimUpdate()
{
  for (u32 i = 0; i < kUsedFSM; ++i) {
    kFSM[i].Update();
  }
}

}  // namespace animation
