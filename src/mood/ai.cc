#pragma once

#include "mood/entity.cc"
#include "physics/physics.cc"

static u32 kAISpawnNum;

template <typename T>
Blackboard*
bb(const T* t) {
  if (!t->blackboard_id) return nullptr;
  return FindBlackboard(t->blackboard_id);
}

struct Patrol {
  // Left and right endpoints of patrol on x axis.
  r32 left_x;
  r32 right_x;
};

void
AICreate(v2f pos, v2f dims, CharacterAIBehavior behavior)
{
  Character* ai_character = UseEntityCharacter(pos, dims);
  physics::Particle2d* ai_particle = FindParticle(ai_character);
  if (ai_particle) {
    ai_particle->collision_mask = kCollisionMaskCharacter;
    ai_particle->damping = 0.005f;
    switch (behavior) {
      case kBehaviorSimple: {
      } break;
      case kBehaviorSimpleFlying: {
        SBIT(ai_particle->flags, physics::kParticleIgnoreGravity);
        SBIT(ai_particle->flags, physics::kParticleIgnoreCollisionResolution);
      } break;
      default: break;
    };
    BB_SET(bb(ai_character), kAIBbType, behavior);
  }

  Patrol patrol;
  patrol.left_x = pos.x - 50.f;
  patrol.right_x = pos.x + 50.f;
  BB_SET(bb(ai_character), kAIBbPatrol, patrol);
}

void
AIInitialize()
{
}

void
AIBehaviorPatrol(Character* c)
{
  const Patrol* patrol;
  if (!BB_GET(bb(c), kAIBbPatrol, patrol)) return;
  physics::Particle2d* ai_particle = FindParticle(c);
  if (fabs(ai_particle->velocity.x) <= FLT_EPSILON) {
    r32 r = math::Random(0.f, 1.f);
    if (r < 0.5f) {
      ai_particle->acceleration.x = -kEnemyAcceleration;
    } else {
      ai_particle->acceleration.x = kEnemyAcceleration;
    }
  }
  if (ai_particle->position.x <= patrol->left_x) {
    ai_particle->acceleration.x = kEnemyAcceleration;
  }

  if (ai_particle->position.x >= patrol->right_x) {
    ai_particle->acceleration.x = -kEnemyAcceleration;
  }
}

void
AIBehaviorSimple(Character* c)
{
  AIBehaviorPatrol(c);
  // Head towards the player I guess?
  //physics::Particle2d* player_particle = PlayerParticle();
  //physics::Particle2d* ai_particle = FindParticle(c);
  //if (player_particle->position.x > ai_particle->position.x) {
  //  ai_particle->acceleration.x = kEnemyAcceleration;
  //} else {
  //  ai_particle->acceleration.x = -kEnemyAcceleration;
  //}
}

void
AIBehaviorFlying(Character* c)
{
  // Head towards the player I guess?
  physics::Particle2d* player_particle = PlayerParticle();
  physics::Particle2d* ai_particle = FindParticle(c);
  ai_particle->acceleration =
    math::Normalize(player_particle->position - ai_particle->position) *
        kEnemyAcceleration;
}

void
AIUpdate()
{
  if (!kEnableEnemies) return;

  FOR_EACH_ENTITY(Character, c, {
    const u32* behavior;
    if (!BB_GET(bb(c),  kAIBbType, behavior)) continue;
    switch (*behavior) {
      case kBehaviorSimple: {
        AIBehaviorSimple(c);
      } break;
      case kBehaviorSimpleFlying: {
        AIBehaviorFlying(c);
      } break;
      default: {
        printf("Unknown behavior entity %u\n", c->id);
      } break;
    }
  });
}
