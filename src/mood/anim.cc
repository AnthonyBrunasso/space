#pragma once

namespace mood
{

enum AdventurerAnimState {
  kAdventurerAnimIdle      = 0,
  kAdventurerAnimWalk      = 1,
  kAdventurerAnimJump      = 2,
  kAdventurerAnimNumStates = 3,
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
          kAdventurerAnimJump,
          [](u32 entity_id) {
            GET_PARTICLE_OR_RETURN(p, entity_id, return false);
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
            GET_PARTICLE_OR_RETURN(p, entity_id, return false);
            return p->velocity.x == 0.f;
          })
      .Transition(
          kAdventurerAnimJump,
          [](u32 entity_id) {
            GET_PARTICLE_OR_RETURN(p, entity_id, return false);
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
