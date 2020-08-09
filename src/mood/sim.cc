#pragma once

#include "mood/projectile.cc"
#include "mood/character.cc"
#include "mood/collision.cc"

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
  kCharacter.player_id = 0;
  for (u32 i = 0; i < kComponentCount; ++i) {
    ecs::GetComponents(i)->Clear();
  }
  ecs::ResetEntity();
  physics::Reset();
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
  // Collisions take place after physics integration step. Collisions must
  // be resolved before rendering.
  CollisionUpdate();

  // Cleanup entities marked to die at the end of each simulation update
  // frame. Particle for physics system will be destroyed at the top of the
  // next integration step.
  ECS_ITR1(itr, kDeathComponent);
  while (itr.Next()) {
    // Just in case an entity got multiple death components.
    if (!itr.e) continue;
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
