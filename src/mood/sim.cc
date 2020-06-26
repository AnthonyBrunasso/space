#pragma once

#include "mood/ai.cc"
#include "mood/entity.cc"
#include "mood/projectile.cc"

#include "physics/physics.cc"

namespace mood {

struct Sim {
  u32 player_id; 
  // Cooldown that dictates whether the player can boost.
  util::Cooldown boost_cooldown;
  // Cooldown that lets player fire weapon.
  util::Cooldown weapon_cooldown;
};

static Sim kSim;

Character* Player() {
  return FindCharacter(kSim.player_id);
}

void
SimInitialize()
{
  Character* player = UseEntityCharacter(v2f(0.f, 0.f), v2f(5.f, 5.f));
  physics::Particle2d* particle = FindParticle(player);
  particle->damping = 0.005f;
  kSim.player_id = player->id;

  physics::CreateInfinteMassParticle2d(v2f(0.f, -5.f), v2f(1000.f, 5.f));

  kSim.boost_cooldown.usec = SECONDS(0.75f);
  util::CooldownInitialize(&kSim.boost_cooldown);

  kSim.weapon_cooldown.usec = SECONDS(0.15f);
  util::CooldownInitialize(&kSim.weapon_cooldown);
  
  AIInitialize();
}

void
__CharacterProjectileCollision(Character* character, Projectile* projectile)
{
  if (character->id == kSim.player_id) return;
  SetDestroyFlag(character);
}

void
__ResolveCollisions()
{
  for (u32 i = 0; i < physics::kUsedBP2dCollision; ++i) {
    physics::BP2dCollision* c = &physics::kBP2dCollision[i];
    // Check for character / projectile collisions.
    Character* character = FindCharacter(c->p1->entity_id);
    Projectile* projectile = FindProjectile(c->p2->entity_id);
    if (!character) character = FindCharacter(c->p2->entity_id);
    if (!projectile) projectile = FindProjectile(c->p1->entity_id);
    if (character && projectile) {
      __CharacterProjectileCollision(character, projectile);
    }
  }
}

void
SimUpdate()
{
  physics::Integrate(kFrameDelta);

  FOR_EACH_ENTITY(Character, c, {
    physics::Particle2d* particle = FindParticle(c);
    if (FLAGGED(c->character_flags, kCharacterFireWeapon)) {
      if (util::CooldownReady(&kSim.weapon_cooldown)) {
        util::CooldownReset(&kSim.weapon_cooldown);
        ProjectileCreate(particle->position, c->facing, kSim.player_id,
                         kProjectileLaser);
      }
    }
    if (IsZero(particle->velocity)) break;
    if (particle->velocity.x > 0) c->facing = v2f(1.0f, 0.f);
    else if (particle->velocity.x < 0) c->facing = v2f(-1.0f, 0.f);
  });

  __ResolveCollisions();

  ProjectileUpdate();
  AIUpdate();
  rgg::CameraSetPositionXY(FindParticle(Player())->position);

  // Cleanup entities marked to die at the end of each simulation update
  // frame. Particle for physics system will be destroyed at the top of the
  // next integration step.
  for (u32 i = 0; i < kUsedEntity;) {
    Entity* entity = &kEntity[i];
    if (FLAGGED(entity->flags, kEntityDestroy)) {
      SwapEntity(entity->id, kEntity[kUsedEntity - 1].id);
      // Entity array swapped so the entity to delete is now the final.
      physics::DeleteParticle2d(kEntity[kUsedEntity - 1].particle_id);
      ClearEntity(kEntity[kUsedEntity - 1].id);
      continue;
    }
    ++i;
  }
}

}
