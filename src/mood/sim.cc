#pragma once

#include "mood/entity.cc"
#include "mood/projectile.cc"
#include "mood/character.cc"

#include "physics/physics.cc"

namespace mood {

void SpawnerUpdate();  // Defined in spawner.cc
void ObstacleUpdate();  // Defined in obstacle.cc

struct Sim {
  // Frame - incremented when SimUpdate is called.
  u64 frame = 0;
};

static Sim kSim;

static b8 kReloadGame = false;
static char kReloadFrom[64] = "asset/test.map";
static b8 kFreezeGame = false;
static b8 kEnableEnemies = true;

#include "mood/ai.cc"

void
SimInitialize()
{
  CharacterInitialize();
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
__CharacterProjectileCollision(CharacterComponent* character,
                               ProjectileComponent* projectile)
{
  // If the particle created the projectile - ignore the collision.
  if (projectile->from_entity == character->entity_id) return;
  ecs::AssignDeathComponent(projectile->entity_id);
  character->health -= 3.f;
}

void
__ProjectileParticleCollision(ProjectileComponent* projectile,
                              physics::Particle2d* particle,
                              physics::BP2dCollision* c)
{
  ecs::Entity* projectile_ent = ecs::FindEntity(projectile->entity_id);
  ecs::AssignDeathComponent(projectile_ent);
  physics::Particle2d* projectile_particle = physics::FindParticle2d(
      ecs::GetPhysicsComponent(projectile_ent)->particle_id);
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
  if (util::FrameCooldownReady(&kCharacter.player_invulnerable)) {
    player->health -= 1.f;
    util::FrameCooldownReset(&kCharacter.player_invulnerable);
  }
}

void
__PlayerObstacleCollision(Character* player, Obstacle* obstacle)
{
  if (obstacle->obstacle_type == kObstacleBoost) {
    physics::Particle2d* p = FindParticle(player);
    p->force.y = kObstacleBoostForce;
  }
}

void
__ResolveCollisions()
{
#define GETCOMP(Comp, name, e1, e2)                              \
  Comp* name = e1 != nullptr ? ecs::Get##Comp(e1) : nullptr;     \
  if (!name) e2 != nullptr  ? name = ecs::Get##Comp(e2) : nullptr;

#define GETPARTICLE(comp, name, p1, p2)   \
  physics::Particle2d* name = nullptr;    \
  if (p1->entity_id == comp->entity_id) { \
    name = p2;                            \
  } else { name = p1; }

  for (u32 i = 0; i < physics::kUsedBP2dCollision; ++i) {
    physics::BP2dCollision* c = &physics::kBP2dCollision[i];
    ecs::Entity* e1 = ecs::FindEntity(c->p1->entity_id);
    ecs::Entity* e2 = ecs::FindEntity(c->p2->entity_id);

    if ((e1 && e2) &&
        ((e1->Has(kCharacterComponent) && e2->Has(kProjectileComponent)) ||
         (e2->Has(kCharacterComponent) && e1->Has(kProjectileComponent)))) {
      GETCOMP(CharacterComponent, character, e1, e2);
      GETCOMP(ProjectileComponent, projectile, e1, e2);
      if (character && projectile) {
        __CharacterProjectileCollision(character, projectile);
        continue;
      }
    }

    if (((e1 && e1->Has(kProjectileComponent)) ||
         (e2 && e2->Has(kProjectileComponent))) &&
        (c->p1->inverse_mass == 0.f || c->p2->inverse_mass == 0.f)) {
      GETCOMP(ProjectileComponent, projectile, e1, e2);
      GETPARTICLE(projectile, particle, c->p1, c->p2);
      if (projectile && particle) {
        __ProjectileParticleCollision(projectile, particle, c);
        continue;
      }
    }
#if 0
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
    {
      Character* player = FindCharacter(c->p1->entity_id);
      Obstacle* obstacle = FindObstacle(c->p2->entity_id);
      if (player && obstacle) {
        __PlayerObstacleCollision(player, obstacle);
        continue;
      }

      obstacle = FindObstacle(c->p1->entity_id);
      player = FindCharacter(c->p2->entity_id);
      if (player && obstacle) {
        __PlayerObstacleCollision(player, obstacle);
        continue;
      }
    }
#endif
  }
}

bool
SimUpdate()
{
  ++kSim.frame;

  // Reset game if returns true.
  if (CharacterUpdate()) return true;
  ObstacleUpdate();
  SpawnerUpdate();
  ProjectileUpdate();
  AIUpdate();

  ecs::Entity* player = Player();
  if (player) {
    physics::Particle2d* particle =
        physics::FindParticle2d(
            ecs::GetPhysicsComponent(player)->particle_id);
    rgg::CameraLerpToPositionXY(particle->position +
                                v2f(0.f, kCameraYOffset), .1f);
  }

  physics::Integrate(kFrameDelta);
  __ResolveCollisions();

  // Cleanup entities marked to die at the end of each simulation update
  // frame. Particle for physics system will be destroyed at the top of the
  // next integration step.
  ecs::EntityItr<1> itr(kDeathComponent);
  while (itr.Next()) {
    if (itr.e->Has(kBlackboardComponent)) {
      SwapAndClearBlackboard(
          ecs::GetBlackboardComponent(itr.e)->blackboard_id);
    }
    if (itr.e->Has(kPhysicsComponent)) {
      physics::DeleteParticle2d(ecs::GetPhysicsComponent(itr.e)->particle_id);
    }
    ecs::DeleteEntity(itr.e, kComponentCount);
  }
  ecs::GetComponents(kDeathComponent)->Clear();

  // TODO: Not entirely sure why this is needed. Without this - sometimes
  // physics geometry can get into an invalid state where things tunnel
  // through. *shrug*
  physics::BPUpdateAll();

  return false;
}

}
