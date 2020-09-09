#pragma once

namespace mood
{

enum AdventurerAnimState {
  kAdventurerAnimIdle       = 0,
  kAdventurerAnimWalk       = 1,
  kAdventurerAnimSingleJump = 2,
  kAdventurerAnimDoubleJump = 3,
  kAdventurerAnimNumStates  = 4,
};

u32
AnimInitAdventurerFSM(animation::FSM* fsm, u32 entity_id)
{
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
            GET_PARTICLE_OR_RETURN(p, entity_id, return false);
            return fabs(p->velocity.x) > 0.f;
          })
      .Transition(
          kAdventurerAnimSingleJump,
          [](u32 entity_id) {
            GET_PARTICLE_OR_RETURN(p, entity_id, return false);
            return !p->on_ground;
          });

  fsm->Node(kAdventurerAnimWalk)
      .Frame(50.f * 1.f, 37.f, 50.f, 37.f, 6)
      .Frame(50.f * 2.f, 37.f, 50.f, 37.f, 6)
      .Frame(50.f * 3.f, 37.f, 50.f, 37.f, 6)
      .Frame(50.f * 4.f, 37.f, 50.f, 37.f, 6)
      .Frame(50.f * 5.f, 37.f, 50.f, 37.f, 6)
      .Frame(50.f * 6.f, 37.f, 50.f, 37.f, 6)
      .Transition(
          kAdventurerAnimIdle,
          [](u32 entity_id) {
            GET_PARTICLE_OR_RETURN(p, entity_id, return false);
            return p->velocity.x == 0.f;
          })
      .Transition(
          kAdventurerAnimSingleJump,
          [](u32 entity_id) {
            GET_PARTICLE_OR_RETURN(p, entity_id, return false);
            return !p->on_ground;
          });

  fsm->Node(kAdventurerAnimSingleJump)
      .Frame(50.f * 0.f, 37.f * 2.f, 50.f, 37.f, 3)
      .Frame(50.f * 1.f, 37.f * 2.f, 50.f, 37.f, 4)
      .Frame(50.f * 2.f, 37.f * 2.f, 50.f, 37.f, 5)
      .Frame(50.f * 3.f, 37.f * 2.f, 50.f, 37.f, 5)
      .Frame(50.f * 1.f, 37.f * 3.f, 50.f, 37.f, 6)
      .Frame(50.f * 2.f, 37.f * 3.f, 50.f, 37.f, 6)
      .Flag(animation::kFSMNodeStopOnFinalFrame)
      .Transition(
          kAdventurerAnimIdle,
          [](u32 entity_id) {
            GET_PARTICLE_OR_RETURN(p, entity_id, return false);
            return p->on_ground != 0;
          })
      .Transition(
          kAdventurerAnimDoubleJump,
          [](u32 entity_id) {
            ecs::Entity* entity = ecs::FindEntity(entity_id);
            physics::Particle2d* p = GetParticle(entity);
            if (!p) return false;
            CharacterComponent* character =
                ecs::GetCharacterComponent(entity);
            return !p->on_ground &&
                   !FLAGGED(character->character_flags,
                            kCharacterCanDoubleJump);
          });

  fsm->Node(kAdventurerAnimDoubleJump)
      .Frame(50.f * 3.f, 37.f * 2.f, 50.f, 37.f, 5)
      .Frame(50.f * 4.f, 37.f * 2.f, 50.f, 37.f, 5)
      .Frame(50.f * 5.f, 37.f * 2.f, 50.f, 37.f, 5)
      .Frame(50.f * 6.f, 37.f * 2.f, 50.f, 37.f, 5)
      .Frame(50.f * 0.f, 37.f * 3.f, 50.f, 37.f, 6)
      .Frame(50.f * 1.f, 37.f * 3.f, 50.f, 37.f, 6)
      .Frame(50.f * 2.f, 37.f * 3.f, 50.f, 37.f, 6)
      .Flag(animation::kFSMNodeStopOnFinalFrame)
      .Transition(
          kAdventurerAnimIdle,
          [](u32 entity_id) {
            GET_PARTICLE_OR_RETURN(p, entity_id, return false);
            return p->on_ground != 0;
          });

  return fsm->id;
}

void
AnimUpdate()
{
  ECS_ITR1(itr, kAnimComponent);
  while (itr.Next()) {
    itr.c.anim->fsm.Update();
  }
}

}  // namespace animation
