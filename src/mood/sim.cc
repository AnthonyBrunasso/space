#pragma once

#include "mood/ai.cc"
#include "mood/entity.cc"
#include "mood/projectile.cc"

#include "physics/physics.cc"

namespace mood {

void RenderCreateEffect(Rectf rect, v4f color, u32 ttl);

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

  physics::CreateInfinteMassParticle2d(v2f(0.f, -5.f), v2f(500.f, 5.f));
  physics::CreateInfinteMassParticle2d(v2f(-50.f, 15.f), v2f(30.f, 5.f));
  physics::CreateInfinteMassParticle2d(v2f(-10.f, 10.f), v2f(10.f, 5.f));
  physics::CreateInfinteMassParticle2d(v2f(10.f, 10.f), v2f(5.f, 35.f));

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
__ProjectileParticleCollision(
    Projectile* projectile, physics::Particle2d* particle,
    physics::BP2dCollision* c)
{
  SetDestroyFlag(projectile);
}

void
__ResolveCollisions()
{
  for (u32 i = 0; i < physics::kUsedBP2dCollision; ++i) {
    physics::BP2dCollision* c = &physics::kBP2dCollision[i];
    {
      // Check for character / projectile collisions.
      Character* character = FindCharacter(c->p1->entity_id);
      Projectile* projectile = FindProjectile(c->p2->entity_id);
      if (!character) character = FindCharacter(c->p2->entity_id);
      if (!projectile) projectile = FindProjectile(c->p1->entity_id);
      if (character && projectile) {
        __CharacterProjectileCollision(character, projectile);
        continue;
      }
    }
    {
      Projectile* projectile = FindProjectile(c->p1->entity_id);
      physics::Particle2d* particle = c->p2;
      if (!projectile) {
        projectile = FindProjectile(c->p2->entity_id);
        particle = c->p1;
      }
      if (projectile && particle && particle->inverse_mass == 0.f) {
        __ProjectileParticleCollision(projectile, particle, c);
        continue;
      }
    }
  }
}

void
SimUpdate()
{
  physics::Integrate(kFrameDelta);

  FOR_EACH_ENTITY(Character, c, {
    physics::Particle2d* particle = FindParticle(c);
    if (particle) {
      if (FLAGGED(c->character_flags, kCharacterFireWeapon)) {
        if (util::CooldownReady(&kSim.weapon_cooldown)) {
          util::CooldownReset(&kSim.weapon_cooldown);
          ProjectileCreate(particle->position, c->facing, kSim.player_id,
                           kProjectileLaser);
        }
      }
      if (FLAGGED(c->ability_flags, kCharacterAbilityBoost)) {
        if (util::CooldownReady(&kSim.boost_cooldown)) {
          util::CooldownReset(&kSim.boost_cooldown);
          particle->force += c->ability_dir * 10000.f;
          c->trail_effect_ttl = 30;
          //RenderCreateEffect(particle->aabb(), v4f(1.f, 1.f, 1.f, 1.f), 30);
        }
      }
      if (FLAGGED(c->character_flags, kCharacterJump)) {
        if (particle->on_ground) particle->force.y += kJumpForce;
      }
      if (!IsZero(particle->velocity)) {
        if (particle->velocity.x > 0) c->facing = v2f(1.0f, 0.f);
        else if (particle->velocity.x < 0) c->facing = v2f(-1.0f, 0.f);
      }
    }

    if (c->trail_effect_ttl > 0) {
      if (c->trail_effect_ttl % 5 == 0) {
        Rectf effect_rect(particle->aabb());
        effect_rect.x += 1.f;
        effect_rect.y += 1.f;
        effect_rect.width -= 1.f;
        effect_rect.height -= 1.f;
        RenderCreateEffect(effect_rect, v4f(.4f, 1.f, .4f, 0.6f), 15);
      }
      --c->trail_effect_ttl;
    }
  });

  __ResolveCollisions();

  ProjectileUpdate();
  //AIUpdate();
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
