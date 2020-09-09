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
AnimUseAdventurerFSM()
{
  FSM* fsm = UseFSM();
  fsm->Initialize(kAdventurerAnimNumStates, kAdventurerAnimIdle);
  fsm->Node(kAdventurerAnimIdle)
      .Frame(50.f * 0.f, 0.f, 50.f, 37.f, 25)
      .Frame(50.f * 1.f, 0.f, 50.f, 37.f, 25)
      .Frame(50.f * 2.f, 0.f, 50.f, 37.f, 25)
      .Frame(50.f * 3.f, 0.f, 50.f, 37.f, 25)
      .Transition(
          kAdventurerAnimWalk,
          [](ecs::Entity entity) {
            return false;
          })
      .Transition(
          kAdventurerAnimJump,
          [](ecs::Entity entity) {
            return false;
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
