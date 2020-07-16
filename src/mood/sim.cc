#pragma once

#include "mood/entity.cc"
#include "mood/projectile.cc"

#include "physics/physics.cc"

namespace mood {

void RenderCreateEffect(Rectf rect, v4f color, u32 ttl, r32 rotate_delta);

struct Sim {
  u32 player_id; 
  // Cooldown that dictates whether the player can boost.
  util::Cooldown boost_cooldown;
  // Cooldown that lets player fire weapon.
  util::Cooldown weapon_cooldown;
  // Cooldown that makes player invulnerable.
  util::Cooldown player_invulnerable;
};

static Sim kSim;

Character* Player() {
  return FindCharacter(kSim.player_id);
}

physics::Particle2d* PlayerParticle() {
  return physics::FindParticle2d(FindCharacter(kSim.player_id)->particle_id);
}

#include "mood/ai.cc"

void
SimInitialize()
{
  Character* player =
      UseEntityCharacter(v2f(0.f, 60.f), v2f(kPlayerWidth, kPlayerHeight));
  physics::Particle2d* particle = FindParticle(player);
  particle->collision_mask = kCollisionMaskCharacter;
  particle->damping = 0.005f;
  kSim.player_id = player->id;

  SBIT(physics::CreateInfinteMassParticle2d(
           v2f(0.f, 0.f), v2f(1910.f, kPlayerHeight))->user_flags, kParticleCollider);
  //SBIT(physics::CreateInfinteMassParticle2d(
  //         v2f(-50.f, 15.f), v2f(30.f, 5.f))->user_flags, kParticleCollider);
  //SBIT(physics::CreateInfinteMassParticle2d(
  //         v2f(-10.f, 10.f), v2f(10.f, 5.f))->user_flags, kParticleCollider);
  //SBIT(physics::CreateInfinteMassParticle2d(
  //         v2f(10.f, 25.f), v2f(5.f, 35.f))->user_flags, kParticleCollider);

  kSim.boost_cooldown.usec = SECONDS(1.f);
  util::CooldownInitialize(&kSim.boost_cooldown);

  kSim.weapon_cooldown.usec = SECONDS(0.15f);
  util::CooldownInitialize(&kSim.weapon_cooldown);

  kSim.player_invulnerable.usec = SECONDS(0.5f);
  util::CooldownInitialize(&kSim.player_invulnerable);
  
  AIInitialize();
}

void
SimReset()
{
  physics::Reset();
  ResetEntity();
}

void
__CharacterProjectileCollision(Character* character, Projectile* projectile)
{
  if (character->id == kSim.player_id) return;
  SetDestroyFlag(projectile);
  character->health -= 3.f;
}

void
__ProjectileParticleCollision(Projectile* projectile,
                              physics::Particle2d* particle,
                              physics::BP2dCollision* c)
{
  SetDestroyFlag(projectile);
  physics::Particle2d* projectile_particle = FindParticle(projectile);
  // Spawn effect flying off in opposite direction.
  if (projectile_particle) {
    v2f dir = -math::Normalize(projectile_particle->velocity);
    for (int i = 0; i < 4; ++i) {
      physics::Particle2d* ep =
          physics::CreateParticle2d(projectile_particle->position + dir * 1.f,
                                    v2f(kParticleWidth, kParticleHeight));
      ep->collision_mask = kCollisionMaskCharacter;
      v2f fdir = Rotate(dir, math::Random(-45.f, 45.f));
      ep->force = fdir * math::Random(1000.f, 5000.f);
      ep->ttl = kParticleTTL;
      SBIT(ep->user_flags, kParticleSpark);
    }
  }
}

void
__PlayerCharacterCollision(Character* player, Character* character)
{
  if (util::CooldownReady(&kSim.player_invulnerable)) {
    player->health -= 1.f;
    util::CooldownReset(&kSim.player_invulnerable);
  }
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
      Character* player = FindCharacter(c->p1->entity_id);
      Character* character = nullptr;
      if (player == Player()) {
        character = FindCharacter(c->p2->entity_id);
      }
      if (player && character && player != character) {
        __PlayerCharacterCollision(player, character);
        continue;
      }
      player = FindCharacter(c->p2->entity_id);
      character = nullptr;
      if (player == Player()) {
        character = FindCharacter(c->p1->entity_id);
      }
      if (player && character && player != character) {
        __PlayerCharacterCollision(player, character);
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

bool
SimUpdate()
{
  FOR_EACH_ENTITY_P(Character, c, particle, {
    if (particle) {
      if (FLAGGED(c->character_flags, kCharacterFireWeapon)) {
        if (util::CooldownReady(&kSim.weapon_cooldown)) {
          util::CooldownReset(&kSim.weapon_cooldown);
          ProjectileCreate(particle->position, c->facing, kSim.player_id,
                           kProjectileBullet);
        }
      }
      if (FLAGGED(c->ability_flags, kCharacterAbilityBoost)) {
        if (util::CooldownReady(&kSim.boost_cooldown)) {
          util::CooldownReset(&kSim.boost_cooldown);
          // Boosting make player invulnerable briefly.
          util::CooldownReset(&kSim.player_invulnerable);
          particle->force += c->ability_dir * kPlayerBoostForce;
          c->trail_effect_ttl = 30;
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
        RenderCreateEffect(
            effect_rect, v4f(.4f, 1.f, .4f, 0.6f), 25, math::Random(1.f, 10.f));
      }
      --c->trail_effect_ttl;
    }

    if (FLAGGED(c->character_flags, kCharacterAim)) {
      c->aim_dir = math::Rotate(c->aim_dir, c->aim_rotate_delta);
    }

    if (!FLAGGED(c->character_flags, kCharacterAim) &&
        FLAGGED(c->prev_character_flags, kCharacterAim)) {
        physics::Particle2d* test = physics::CreateParticle2d(
            particle->position + v2f(0.f, 2.f), v2f(100.f, 10.f));
        test->ttl = 50;
        physics::Rotate(test, 45.f);
        SBIT(test->flags, physics::kParticleIgnoreCollisionResolution);
        SBIT(test->flags, physics::kParticleIgnoreGravity);
        SBIT(test->user_flags, kParticleTest);
    }

    if (c->health <= 0.f) {
      if (c == Player()) {
        return true;
      } else {
        SetDestroyFlag(c);
        v2f up(0.f, 1.f);
        for (int i = 0; i < 30; ++i) {
          physics::Particle2d* ep =
              physics::CreateParticle2d(particle->position + up * 1.f,
                                        v2f(kParticleWidth, kParticleHeight));
          ep->collision_mask = kCollisionMaskCharacter;
          v2f dir = Rotate(up, math::Random(-55.f, 55.f));
          ep->force = dir * math::Random(10000.f, 30000.f);
          SBIT(ep->user_flags, kParticleBlood);
          ep->ttl = kParticleTTL;
        }
      }
    }

    c->prev_character_flags = c->character_flags;
  });

  ProjectileUpdate();
  AIUpdate();
  rgg::CameraSetPositionXY(FindParticle(Player())->position +
                           v2f(0.f, kCameraYOffset));

  physics::Integrate(kFrameDelta);
  __ResolveCollisions();

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

  return false;
}

}
