#pragma once

#include "physics/physics.cc"

static u32 kAISpawnNum;

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
  physics::Particle2d* particle =
      physics::CreateParticle2d(pos, dims, ai_entity->id);
  physics_comp->particle_id = particle->id;
  AIComponent* ai_comp = ecs::AssignAIComponent(ai_entity);
  if (particle) {
    SBIT(particle->collision_mask, kCollisionMaskAI);
    SBIT(particle->collision_mask, kCollisionMaskCharacter);
    particle->damping = 0.005f;
    switch (behavior) {
      case kBehaviorSimple: {
      } break;
      case kBehaviorSimpleFlying: {
        SBIT(particle->flags, physics::kParticleIgnoreGravity);
        SBIT(particle->flags, physics::kParticleIgnoreCollisionResolution);
        AnimComponent* anim_comp = ecs::AssignAnimComponent(ai_entity);
        AnimInitFireSpiritFSM(&anim_comp->fsm);
      } break;
      default: break;
    };
    ProjectileWeaponComponent* weapon =
        ecs::AssignProjectileWeaponComponent(ai_entity);
    weapon->projectile_type = kProjectileBullet;
    weapon->projectile_speed = kProjectileSpeed / 4.f;
    weapon->projectile_ttl = 500;
    weapon->cooldown.frame = 60;
    weapon->random_aim_offset = 15.f;
    util::FrameCooldownInitialize(&weapon->cooldown);
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
}

void
AIBehaviorPatrol(AIComponent* ai)
{
  const Patrol* patrol;
  if (!BB_GET(ai->blackboard, kAIBbPatrol, patrol)) return;
  ecs::Entity* entity = ecs::FindEntity(ai->entity_id);
  physics::Particle2d* ai_particle = physics::FindParticle2d(
      ecs::GetPhysicsComponent(entity)->particle_id); 
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
AIBehaviorSimple(AIComponent* ai)
{
  AIBehaviorPatrol(ai);
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
AIBehaviorFlying(AIComponent* ai)
{
  // Head towards the player I guess?
  physics::Particle2d* player_particle = PlayerParticle();
  if (!player_particle) return;
  ecs::Entity* entity = ecs::FindEntity(ai->entity_id);
  physics::Particle2d* ai_particle = physics::FindParticle2d(
      ecs::GetPhysicsComponent(entity)->particle_id); 
  CharacterComponent* c = ecs::GetCharacterComponent(entity);
  v2f dir = math::Normalize(player_particle->position - ai_particle->position);
  ai_particle->acceleration = dir * kEnemyAcceleration;
  if (math::LengthSquared(
      player_particle->position - ai_particle->position) < 50000.f) {
    c->aim_dir = dir;
    SBIT(c->character_flags, kCharacterFireWeapon);
  } else {
    CBIT(c->character_flags, kCharacterFireWeapon);
  }
}

void
AIUpdate()
{
  if (!kEnableEnemies) return;
  ECS_ITR1(itr, kAIComponent);
  while (itr.Next()) {
    const u32* behavior;
    if (!BB_GET(itr.c.ai->blackboard,  kAIBbType, behavior)) continue;
    switch (*behavior) {
      case kBehaviorSimple: {
        AIBehaviorSimple(itr.c.ai);
      } break;
      case kBehaviorSimpleFlying: {
        AIBehaviorFlying(itr.c.ai);
      } break;
      default: {
        printf("Unknown behavior entity %u\n", itr.e->id);
      } break;
    }
  }
}
