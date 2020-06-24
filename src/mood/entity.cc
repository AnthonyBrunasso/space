#pragma once

#include "common/common.cc"
#include "physics/physics.cc"
#include "util/cooldown.cc"

namespace mood {

// Used to determine specific collision responses.
enum PhysicsFlags {
  kPhysicsCharacter = 0,
  kPhysicsProjectile = 0
};

struct Player {
  u32 particle_id;
};

DECLARE_ARRAY(Player, 1);

physics::Particle2d* PlayerParticle(uint32_t idx) {
  if (idx >= kUsedPlayer) return nullptr;
  return physics::FindParticle2d(kPlayer[idx].particle_id);
}

enum ProjectileType {
  PROJECTILE_LAZER = 0,
};

struct Projectile {
  u32 particle_id;
  v2f dir;
  ProjectileType type;
  // Number of updates the projectile should live for.
  uint64_t updates_to_live = 0;
};

DECLARE_ARRAY(Projectile, 128);

physics::Particle2d* ProjectileParticle(uint32_t idx) {
  if (idx >= kUsedProjectile) return nullptr;
  return physics::FindParticle2d(kProjectile[idx].particle_id);
}

enum CharacterFlags {
  // Set when a character gets killed.
  kCharacterDeath = 0,
};

struct Character {
  u32 particle_id;
  u32 flags;
};

DECLARE_ARRAY(Character, 64);

}
