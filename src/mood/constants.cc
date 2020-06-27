#pragma once

namespace mood {

constexpr r32 kFrameDelta  = 0.016666f;
constexpr r32 kJumpForce = 12000.f;
constexpr r32 kPlayerAcceleration = 200.f;

enum CollisionMask {
  // Collide with everything.
  kCollisionMaskNone = 0,
  // Characters do not resolve collisions amongst each other.
  kCollisionMaskCharacter = 1,
};

enum ParticleFlags {
  kParticleBlood = 0
};

}
