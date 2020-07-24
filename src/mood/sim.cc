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

static b8 kReloadGame = false;
static char kReloadFrom[64] = "asset/test.map";
static b8 kFreezeGame = false;
static b8 kEnableEnemies = false;

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

  kSim.boost_cooldown.usec = SECONDS(1.f);
  util::CooldownInitialize(&kSim.boost_cooldown);

  kSim.weapon_cooldown.usec = SECONDS(0.15f);
  util::CooldownInitialize(&kSim.weapon_cooldown);

  kSim.player_invulnerable.usec = SECONDS(0.5f);
  util::CooldownInitialize(&kSim.player_invulnerable);
  
  AIInitialize();

  // After initialization game is reloaded.
  kReloadGame = false;
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
      v2f fdir = Rotate(dir, math::Random(-25.f, 25.f));
      ep->force = fdir * math::Random(3000.f, 15000.f);
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
          ProjectileCreate(particle->position + v2f(0.f, 0.f), c->aim_dir,
                           kSim.player_id, kProjectileBullet);
        }
      }
      if (FLAGGED(c->character_flags, kCharacterFireSecondary)) {
        if (util::CooldownReady(&kSim.weapon_cooldown)) {
          util::CooldownReset(&kSim.weapon_cooldown);
          ProjectileCreate(particle->position + v2f(0.f, 0.f), c->aim_dir,
                           kSim.player_id, kProjectileGrenade);
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

    c->aim_dir =
        math::Normalize(math::Rotate(c->aim_dir, c->aim_rotate_delta));

    if (c == Player()) {
      Character* player = c;
      if (player->facing.x > 0.f) {
        r32 angle = atan2(player->aim_dir.y, player->aim_dir.x) * 180.f / PI;
        if (angle > kAimAngleClamp || angle < -kAimAngleClamp) {
          angle = CLAMPF(angle, -kAimAngleClamp, kAimAngleClamp);
          player->aim_dir = math::Rotate(v2f(1.f, 0.f), angle);
        } 
      } else if (player->facing.x < 0.f) {
        r32 angle = math::Atan2(player->aim_dir.y, player->aim_dir.x);
        r32 low = 180.f - kAimAngleClamp;
        r32 high = 180.f + kAimAngleClamp;
        if (angle > kAimAngleClamp || angle < -kAimAngleClamp) {
          angle = CLAMPF(angle, low, high);
          player->aim_dir = math::Rotate(v2f(1.f, 0.f), angle);
        }
      }
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

    if (particle->velocity.x > 0.f) {
      c->facing.x = 1.f;
      c->aim_dir.x = fabs(c->aim_dir.x);
    }
    if (particle->velocity.x < 0.f) {
      c->facing.x = -1.f;
      c->aim_dir.x = -1.f * fabs(c->aim_dir.x);
    }
    // Particles on the ground with no acceleration should stop immediately.
    // Without this stuff feels kinda floaty.
    if (particle->on_ground && particle->acceleration.x == 0.f) {
      particle->velocity.x = 0.f;
    }

    c->prev_character_flags = c->character_flags;
  });

  ProjectileUpdate();
  AIUpdate();
  rgg::CameraLerpToPositionXY(FindParticle(Player())->position +
                              v2f(0.f, kCameraYOffset), .1f);

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
