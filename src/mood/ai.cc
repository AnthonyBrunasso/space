#pragma once

#include "mood/entity.cc"
#include "physics/physics.cc"

struct AI {
  // Cooldown for when to spawn an enemy.
  util::Cooldown enemy_cooldown;
};

static AI kAI;

void
AICreate(v2f pos, v2f dims)
{
  Character* ai_character = UseEntityCharacter(pos, dims);
  physics::Particle2d* ai_particle = FindParticle(ai_character);
  if (ai_particle) {
    ai_particle->collision_mask = kCollisionMaskCharacter;
    ai_particle->damping = 0.005f;
    u32 behavior = kBehaviorSimple;
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
  constexpr r32 kSimpleAcceleration = 50.f;
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
AIUpdate()
{
  if (util::CooldownReady(&kAI.enemy_cooldown)) {
    physics::Particle2d* player_particle = PlayerParticle();
    v2f spawn(math::Random(-100.f, 100.f), 10.f);
    if (Length(spawn - player_particle->position) < 10.f) {
      spawn = v2f(math::Random(-100.f, 100.f), 10.f);
    }
    AICreate(spawn, v2f(3.5f, 5.0f));
    util::CooldownReset(&kAI.enemy_cooldown);
  }

  FOR_EACH_ENTITY(Character, c, {
    const u32* behavior;
    if (!BB_GET(c->bb,  kAIBbType, behavior)) continue;
    switch (*behavior) {
      case kBehaviorSimple: {
        AIBehaviorSimple(c);
      } break;
      default: {
        printf("Unknown behavior entity %u\n", c->id);
      } break;
    }
  });
}
