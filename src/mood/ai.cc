#pragma once

#include "mood/entity.cc"
#include "physics/physics.cc"

static u32 kAISpawnNum;

struct AI {
  util::FrameCooldown spawn_cooldown;
};

static AI kAI;


struct Patrol {
  // Left and right endpoints of patrol on x axis.
  r32 left_x;
  r32 right_x;
};

void
AICreate(v2f pos, v2f dims, CharacterAIBehavior behavior)
{
  //Character* ai_character = UseEntityCharacter(pos, dims);
  //ai_character->blackboard_id = UseBlackboard()->id;
  //physics::Particle2d* ai_particle = FindParticle(ai_character);
  ecs::Entity* ai_entity = ecs::UseEntity();
  CharacterComponent* ai_character = ecs::AssignCharacterComponent(ai_entity);
  PhysicsComponent* physics_comp = ecs::AssignPhysicsComponent(ai_entity);
  physics::Particle2d* particle =  physics::CreateParticle2d(pos, dims);
  physics_comp->particle_id = particle->id;
  AIComponent* ai_comp = ecs::AssignAIComponent(ai_entity);
  if (particle) {
    particle->collision_mask = kCollisionMaskCharacter;
    particle->damping = 0.005f;
    switch (behavior) {
      case kBehaviorSimple: {
      } break;
      case kBehaviorSimpleFlying: {
        SBIT(particle->flags, physics::kParticleIgnoreGravity);
        SBIT(particle->flags, physics::kParticleIgnoreCollisionResolution);
      } break;
      default: break;
    };
    ai_character->weapon_cooldown.frame = 50;
    util::FrameCooldownInitialize(&ai_character->weapon_cooldown);
    BB_SET(ai_comp->blackboard, kAIBbType, behavior);
  }

  Patrol patrol;
  patrol.left_x = pos.x - 50.f;
  patrol.right_x = pos.x + 50.f;
  BB_SET(ai_comp->blackboard, kAIBbPatrol, patrol);
}

void
AIInitialize()
{
  kAI.spawn_cooldown.frame = 300;
  util::FrameCooldownInitialize(&kAI.spawn_cooldown);
}

void
AIBehaviorPatrol(Character* c)
{
  const Patrol* patrol;
  //if (!BB_GET(bb(c), kAIBbPatrol, patrol)) return;
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
  v2f dir = math::Normalize(player_particle->position - ai_particle->position);
  ai_particle->acceleration = dir * kEnemyAcceleration;
  if (math::LengthSquared(
      player_particle->position - ai_particle->position) < 50000.f) {
    c->aim_dir = math::Rotate(dir, math::Random(-20.f, 20.f));
    SBIT(c->character_flags, kCharacterFireWeapon);
  } else {
    CBIT(c->character_flags, kCharacterFireWeapon);
  }
}

void
AIUpdate()
{
  if (!kEnableEnemies) return;
#if 0
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

  if (util::FrameCooldownReady(&kAI.spawn_cooldown)) {
    util::FrameCooldownReset(&kAI.spawn_cooldown);
    AICreate(v2f(math::Random(-100.f, 100.f), math::Random(100.f, 200.f)),
             v2f(15.f, 15.f), kBehaviorSimpleFlying);
  }
#endif
}
