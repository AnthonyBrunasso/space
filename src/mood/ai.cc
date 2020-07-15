#pragma once

#include "mood/entity.cc"
#include "physics/physics.cc"

struct AI {
  // Cooldown for when to spawn an enemy.
  util::Cooldown enemy_cooldown;
};

static AI kAI;

static u32 kAISpawnNum;

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
    BB_SET(ai_character->bb, kAIBbType, behavior);
  }
}

void
AIInitialize()
{
  kAI.enemy_cooldown.usec = SECONDS(3.f);
  util::CooldownInitialize(&kAI.enemy_cooldown);
}

void
AIBehaviorSimple(Character* c)
{
  constexpr r32 kSimpleAcceleration = 150.f;
  // Head towards the player I guess?
  physics::Particle2d* player_particle = PlayerParticle();
  physics::Particle2d* ai_particle = FindParticle(c);
  if (player_particle->position.x > ai_particle->position.x) {
    ai_particle->acceleration.x = kSimpleAcceleration;
  } else {
    ai_particle->acceleration.x = -kSimpleAcceleration;
  }
}

void
AIBehaviorFlying(Character* c)
{
  constexpr r32 kSimpleAcceleration = 150.f;
  // Head towards the player I guess?
  physics::Particle2d* player_particle = PlayerParticle();
  physics::Particle2d* ai_particle = FindParticle(c);
  ai_particle->acceleration =
    math::Normalize(player_particle->position - ai_particle->position) *
        kSimpleAcceleration;
}

void
AIUpdate()
{
  if (util::CooldownReady(&kAI.enemy_cooldown)) {

    physics::Particle2d* player_particle = PlayerParticle();

    v2f spawn = {};
    CharacterAIBehavior behavior;
    //if (kAISpawnNum % 2) { 
      spawn = v2f(math::Random(-1000.f, 1000.f), 70.f);
      while (Length(spawn - player_particle->position) < 10.f) {
        spawn = v2f(math::Random(-1000.f, 1000.f), 70.f);
      }
      behavior = kBehaviorSimple;
    //} else {
    //  spawn = v2f(math::Random(-100.f, 100.f), math::Random(40.f, 100.f));
    //  while (Length(spawn - player_particle->position) < 10.f) {
    //    spawn = v2f(math::Random(-100.f, 100.f), math::Random(40.f, 100.f));
    //  }
    //  behavior = kBehaviorSimpleFlying;
    //}
    AICreate(spawn, v2f(kEnemySnailWidth, kEnemySnailHeight), behavior);
    util::CooldownReset(&kAI.enemy_cooldown);
    ++kAISpawnNum;
  }

  FOR_EACH_ENTITY(Character, c, {
    const u32* behavior;
    if (!BB_GET(c->bb,  kAIBbType, behavior)) continue;
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
