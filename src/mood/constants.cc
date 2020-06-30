#pragma once

namespace mood {

constexpr r32 kFrameDelta  = 0.016666f;
constexpr r32 kJumpForce = 12000.f;
constexpr r32 kPlayerAcceleration = 200.f;
constexpr r32 kPlayerHealthBarWidth = 400.f;
constexpr r32 kPlayerHealthBarHeight = 20.f;

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
