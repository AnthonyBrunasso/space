#pragma once

namespace mood
{

enum AdventurerAnimState {
  kAdventurerAnimIdle        = 0,
  kAdventurerAnimWalk        = 1,
  kAdventurerAnimSingleJump  = 2,
  kAdventurerAnimDoubleJump  = 3,
  kAdventurerAnimAttackOne   = 4,
  kAdventurerAnimAttackTwo   = 5,
  kAdventurerAnimAttackThree = 6,
  kAdventurerAnimNumStates   = 7,
};

static r32 kAdventurerWidth = 50.f;
static r32 kAdventurerHeight = 37.f;

enum FireSpiritAnimState {
  kFireSpiritAnimIdle = 0,
  kFireSpiritAnimNumStates = 1,
};

static r32 kFireSpiritWidth = 62.f;
static r32 kFireSpiritHeight = 43.f;

bool EntityInAir(u32 entity_id) {
  GET_PARTICLE_OR_RETURN(p, entity_id, return false);
  return !p->on_ground;
}

bool EntityOnGround(u32 entity_id) {
  GET_PARTICLE_OR_RETURN(p, entity_id, return false);
  return p->on_ground != 0;
}

bool EntityAttacking(u32 entity_id) {
  ecs::Entity* entity = ecs::FindEntity(entity_id);
  if (!entity) return false;
  CharacterComponent* character = ecs::GetCharacterComponent(entity);
  return FLAGGED(character->character_flags, kCharacterAttackMelee);
}

void
AnimInitAdventurerFSM(animation::FSM* fsm)
{
  fsm->Initialize(kAdventurerAnimNumStates, kAdventurerAnimIdle);

  fsm->Node(kAdventurerAnimIdle)
      .Frame(kAdventurerWidth * 0.f, 0.f, kAdventurerWidth, kAdventurerHeight, 25)
      .Frame(kAdventurerWidth * 1.f, 0.f, kAdventurerWidth, kAdventurerHeight, 25)
      .Frame(kAdventurerWidth * 2.f, 0.f, kAdventurerWidth, kAdventurerHeight, 25)
      .Frame(kAdventurerWidth * 3.f, 0.f, kAdventurerWidth, kAdventurerHeight, 25)
      .Transition(kAdventurerAnimAttackTwo, EntityAttacking)
      .Transition(
          kAdventurerAnimWalk,
          [](u32 entity_id) {
            GET_PARTICLE_OR_RETURN(p, entity_id, return false);
            return fabs(p->velocity.x) > 0.f;
          })
      .Transition(kAdventurerAnimSingleJump, EntityInAir);

  fsm->Node(kAdventurerAnimWalk)
      .Frame(kAdventurerWidth * 1.f, kAdventurerHeight, kAdventurerWidth, kAdventurerHeight, 6)
      .Frame(kAdventurerWidth * 2.f, kAdventurerHeight, kAdventurerWidth, kAdventurerHeight, 6)
      .Frame(kAdventurerWidth * 3.f, kAdventurerHeight, kAdventurerWidth, kAdventurerHeight, 6)
      .Frame(kAdventurerWidth * 4.f, kAdventurerHeight, kAdventurerWidth, kAdventurerHeight, 6)
      .Frame(kAdventurerWidth * 5.f, kAdventurerHeight, kAdventurerWidth, kAdventurerHeight, 6)
      .Frame(kAdventurerWidth * 6.f, kAdventurerHeight, kAdventurerWidth, kAdventurerHeight, 6)
      .Transition(kAdventurerAnimAttackTwo, EntityAttacking)
      .Transition(
          kAdventurerAnimIdle,
          [](u32 entity_id) {
            GET_PARTICLE_OR_RETURN(p, entity_id, return false);
            return p->velocity.x == 0.f;
          })
      .Transition(kAdventurerAnimSingleJump, EntityInAir);

  fsm->Node(kAdventurerAnimSingleJump)
      .Frame(kAdventurerWidth * 2.f, kAdventurerHeight * 2.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 3.f, kAdventurerHeight * 2.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 1.f, kAdventurerHeight * 3.f, kAdventurerWidth, kAdventurerHeight, 6)
      .Frame(kAdventurerWidth * 2.f, kAdventurerHeight * 3.f, kAdventurerWidth, kAdventurerHeight, 6)
      .Flag(animation::kFSMNodeStopOnFinalFrame)
      .Transition(kAdventurerAnimAttackTwo, EntityAttacking)
      .Transition(kAdventurerAnimIdle, EntityOnGround)
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
      .Frame(kAdventurerWidth * 3.f, kAdventurerHeight * 2.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 4.f, kAdventurerHeight * 2.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 5.f, kAdventurerHeight * 2.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 6.f, kAdventurerHeight * 2.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 0.f, kAdventurerHeight * 3.f, kAdventurerWidth, kAdventurerHeight, 6)
      .Frame(kAdventurerWidth * 1.f, kAdventurerHeight * 3.f, kAdventurerWidth, kAdventurerHeight, 6)
      .Frame(kAdventurerWidth * 2.f, kAdventurerHeight * 3.f, kAdventurerWidth, kAdventurerHeight, 6)
      .Flag(animation::kFSMNodeStopOnFinalFrame)
      .Transition(kAdventurerAnimAttackTwo, EntityAttacking)
      .Transition(kAdventurerAnimIdle, EntityOnGround);

  fsm->Node(kAdventurerAnimAttackOne)
      .Frame(kAdventurerWidth * 0.f, kAdventurerHeight * 6.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 1.f, kAdventurerHeight * 6.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 2.f, kAdventurerHeight * 6.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 3.f, kAdventurerHeight * 6.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 4.f, kAdventurerHeight * 6.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 5.f, kAdventurerHeight * 6.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 6.f, kAdventurerHeight * 6.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Flag(animation::kFSMNodePlayUntilComplete)
      .Flag(animation::kFSMNodeCantMove)
      .Transition(kAdventurerAnimAttackTwo, EntityAttacking)
      .Transition(kAdventurerAnimIdle, EntityOnGround)
      .Transition(kAdventurerAnimSingleJump, EntityInAir);

  fsm->Node(kAdventurerAnimAttackTwo)
      .Frame(kAdventurerWidth * 0.f, kAdventurerHeight * 7.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 1.f, kAdventurerHeight * 7.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 2.f, kAdventurerHeight * 7.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 3.f, kAdventurerHeight * 7.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Flag(animation::kFSMNodePlayUntilComplete)
      .Flag(animation::kFSMNodeCantMove)
      .Transition(kAdventurerAnimAttackThree, EntityAttacking)
      .Transition(kAdventurerAnimIdle, EntityOnGround)
      .Transition(kAdventurerAnimSingleJump, EntityInAir);

  fsm->Node(kAdventurerAnimAttackThree)
      .Frame(kAdventurerWidth * 4.f, kAdventurerHeight * 7.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 5.f, kAdventurerHeight * 7.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 6.f, kAdventurerHeight * 7.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 0.f, kAdventurerHeight * 8.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 1.f, kAdventurerHeight * 8.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Frame(kAdventurerWidth * 2.f, kAdventurerHeight * 8.f, kAdventurerWidth, kAdventurerHeight, 5)
      .Flag(animation::kFSMNodePlayUntilComplete)
      .Flag(animation::kFSMNodeCantMove)
      .Transition(kAdventurerAnimIdle, EntityOnGround)
      .Transition(kAdventurerAnimSingleJump, EntityInAir);
}

void
AnimInitFireSpiritFSM(animation::FSM* fsm)
{
  fsm->Initialize(kFireSpiritAnimNumStates, kFireSpiritAnimIdle);

  fsm->Node(kAdventurerAnimIdle)
      .Frame(kFireSpiritWidth * 0.f, 0.f, kFireSpiritWidth, kFireSpiritHeight, 25)
      .Frame(kFireSpiritWidth * 1.f, 0.f, kFireSpiritWidth, kFireSpiritHeight, 25)
      .Frame(kFireSpiritWidth * 2.f, 0.f, kFireSpiritWidth, kFireSpiritHeight, 25)
      .Frame(kFireSpiritWidth * 3.f, 0.f, kFireSpiritWidth, kFireSpiritHeight, 25);
}

void
AnimUpdate()
{
  ECS_ITR1(itr, kAnimComponent);
  while (itr.Next()) {
    itr.c.anim->fsm.Update(itr.e->id);
  }
}

}  // namespace animation
