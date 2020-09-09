#pragma once

namespace mood {

constexpr r32 kFrameDelta  = 0.016666f;

// Player constants.
constexpr r32 kJumpForce = 50000.f;
constexpr r32 kPlayerAcceleration = 1500.f;
constexpr r32 kPlayerHealthBarWidth = 100.f;
constexpr r32 kPlayerHealthBarHeight = 5.f;
constexpr r32 kPlayerWidth = 10.f;
constexpr r32 kPlayerHeight = 34.f;
constexpr r32 kPlayerBoostForce = 55000.f;
constexpr r32 kAimAngleClamp = 70.f;
constexpr r32 kAimSensitivity = 3.f;

// Projectile constants.
constexpr r32 kProjectileSpeed = 700.f;
constexpr u32 kProjectileTtl = 50;
constexpr r32 kGrenadeSpeed = 700.f;

// Enemy constants.
constexpr r32 kEnemyHealthHeight = 5.f;
constexpr r32 kEnemyHealthWidthPercent = 1.f;
constexpr r32 kEnemySnailWidth = 28.f;
constexpr r32 kEnemySnailHeight = 28.f;
constexpr r32 kEnemyAcceleration = 350.f;

// Particle constants.
constexpr r32 kParticleWidth = 1.5f;
constexpr r32 kParticleHeight = 1.5f;
constexpr r32 kParticleTTL = 50;

// Camera constants.
constexpr r32 kCameraYOffset = 30.f;

// Obstacle constants.
constexpr r32 kObstacleBoostForce = 8000.f;

// Render target constants.
constexpr r32 kRenderTargetWidth = 800;
constexpr r32 kRenderTargetHeight = 450;
#ifdef __APPLE__
constexpr r32 kScreenWidth = 1440;
constexpr r32 kScreenHeight = 810;
#else
constexpr r32 kScreenWidth = 1920;
constexpr r32 kScreenHeight = 1080;
#endif

// Tilemap constants.
constexpr r32 kTileWidth = 16.f;
constexpr r32 kTileHeight = 16.f;

enum CollisionMask {
  // Collide with everything.
  kCollisionMaskNone = 0,
  // Characters do not resolve collisions amongst each other.
  kCollisionMaskCharacter = 1,
  // AI Stuff we don't want colliding together.
  kCollisionMaskAI = 2,
};

enum ParticleFlags {
  // Blood particles are created when characters explode.
  kParticleBlood = 0,
  // Spark particles are created when projectils hit walls.
  kParticleSpark = 1,
  // These are the static geometry used for collision in the world.
  kParticleCollider = 2,
  // The particle is a spawner of stuff'n'things!!!!!
  kParticleSpawner = 3,
  // The particle boosts the player if they intersect with it.
  kParticleBoost = 4,
};

enum CharacterFlags {
  // If set the character will attempt to fire their primary weapon.
  kCharacterFireWeapon = 0,
  // If set the character will attempt to jump.
  kCharacterJump = 1,
  // Set for the frame the player fired their weapon.
  kCharacterWeaponFired = 2,
  // If set the character will attempt to fire their secondary weapon.
  kCharacterFireSecondary = 3,
  // Set if the character should be moving.
  kCharacterMove = 4,
  // Set if the player is allowed to double jump.
  kCharacterCanDoubleJump = 5,
  // If set the character will attempt to attack with a melee weapon.
  kCharacterAttackMelee = 6,
};

enum CharacterAbilityFlags {
  // If set the character will boost in the direction of its velocity.
  kCharacterAbilityBoost = 0,
};

enum CharacterBbEntry {
  kAIBbType = 0, // Stores value of CharacterAIBehavior.
  kAIBbPatrol = 1,
};

enum CharacterAIBehavior {
  kBehaviorNull = 0,   // Blackboard entries can't be 0.
  kBehaviorSimple = 1,
  kBehaviorSimpleFlying = 2,
};

enum ProjectileType {
  kProjectileLaser = 0,
  kProjectileBullet = 1,
  kProjectileGrenade = 2,
  kProjectilePellet = 3,
};

enum SpawnerType {
  kSpawnerNone = 0,
  kSpawnerPlayer = 1,
  kSpawnerSnail = 2,
  kSpawnerFlying = 3,
};


enum ObstacleType {
  kObstacleNone = 0,
  // Player will boost up if standing on a boost.
  kObstacleBoost = 1,
};

}
