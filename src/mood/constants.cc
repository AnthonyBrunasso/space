#pragma once

namespace mood {

constexpr r32 kFrameDelta  = 0.016666f;

// Player constants.
constexpr r32 kJumpForce = 40000.f;
constexpr r32 kPlayerAcceleration = 1000.f;
constexpr r32 kPlayerHealthBarWidth = 100.f;
constexpr r32 kPlayerHealthBarHeight = 5.f;
constexpr r32 kPlayerWidth = 24.f;
constexpr r32 kPlayerHeight = 24.f;
constexpr r32 kPlayerBoostForce = 25000.f;

// Projectile constants.
constexpr r32 kProjectileSpeed = 700.f;

// Enemy constants.
constexpr r32 kEnemyHealthHeight = 5.f;
constexpr r32 kEnemyHealthWidthPercent = 1.f;
constexpr r32 kEnemySnailWidth = 28.f;
constexpr r32 kEnemySnailHeight = 28.f;

// Particle constants.
constexpr r32 kParticleWidth = 1.5f;
constexpr r32 kParticleHeight = 1.5f;
constexpr r32 kParticleTTL = 50;

// Camera constants.
constexpr r32 kCameraYOffset = 30.f;

// Render target constants.
constexpr r32 kRenderTargetWidth = 640;
constexpr r32 kRenderTargetHeight = 360;
constexpr r32 kScreenWidth = 1920;
constexpr r32 kScreenHeight = 1080;

enum CollisionMask {
  // Collide with everything.
  kCollisionMaskNone = 0,
  // Characters do not resolve collisions amongst each other.
  kCollisionMaskCharacter = 1,
};

enum ParticleFlags {
  // Blood particles are created when characters explode.
  kParticleBlood = 0,
  // Spark particles are created when projectils hit walls.
  kParticleSpark = 1,
  // These are the static geometry used for collision in the world.
  kParticleCollider = 2,
  // These are particles that I simply want to render for testing.
  kParticleTest = 3,
};

}
