#pragma once


#include "common/common.cc"
#include "math/vec.h"
#include "math/rect.h"

namespace physics {

enum PhysicsFlags {
  // Gravity applied along negative y-axis to all particles.
  kGravity = 0,
  // Drag applied in the opposite direction of velocity vector.
  kDrag = 1,
};

struct Particle2d {
  u32 flags = 0;
  v2f position;        // Position of particle - center of aabb.
  v2f velocity;
  v2f acceleration;
  // Inverse mass of this particle. Inverse infinite mass can be reprented
  // with a value of 0 and since 0 mass objects have no physical
  // representation.
  r32 inverse_mass = 1.f;
  r32 damping = 0.05f; // Damping force applied to particle to slow it down.
  v2f force = {};      // Sum of all forces acting on this particle.
  v2f dims;            // Used to create the aabb.

  // Particles keep forward and backward pointers so they can quickly sort
  // themselves after updating.
  Particle2d* next_p2d_x = nullptr;
  Particle2d* prev_p2d_x = nullptr;

  Rectf
  aabb() const
  {
    return Rectf(position - dims / 2.f, dims);
  }

  r32
  mass() const
  {
    return inverse_mass > FLT_EPSILON ? 1.f / inverse_mass : FLT_MAX;
  }
};


struct Physics {
  u32 flags;
  // Force of gravity.
  r32 gravity = 150.f;

  // Linked list in sorted order on x / y axis for collision checks.
  Particle2d* p2d_head_x = nullptr;
};

static Physics kPhysics;

enum ParticleFlags {
  // Ignores the force of gravity if it's enabled.
  kParticleIgnoreGravity = 0,
  // If set the particle will not run its integration step.
  kParticleFreeze = 1,
  // If set the particle will be removed at the beginning of the next
  // integration step.
  kParticleRemove = 2,
};

DECLARE_ARRAY(Particle2d, PHYSICS_PARTICLE_COUNT);

typedef void ApplyForceCallback(Particle2d* p);

struct ForceGenerator {
  ApplyForceCallback* apply_force = nullptr;
};

DECLARE_ARRAY(ForceGenerator, 8);

#include "broadphase.cc"

Particle2d*
CreateParticle2d(v2f pos, v2f dims)
{
  Particle2d* particle = UseParticle2d();
  particle->position = pos;
  particle->dims = dims;
  BPInitP2d(particle);
  return particle;
}

void
DeleteParticle2d(Particle2d* p)
{
  if (!p) return;
  SBIT(p->flags, physics::kParticleRemove);
}

void
ApplyGravity(Particle2d* p)
{
  if (p->inverse_mass <= 0.f) return;
  if (FLAGGED(p->flags, kParticleIgnoreGravity)) return;
  p->force += v2f(0.f, -1.f) * kPhysics.gravity * p->mass();
}

void
CreateGravityGenerator()
{
  ForceGenerator* fg = UseForceGenerator();
  fg->apply_force = ApplyGravity;
}

void
Initialize(u32 physics_flags)
{
  kPhysics.flags = physics_flags;
  if (FLAGGED(kPhysics.flags, kGravity)) {
    CreateGravityGenerator();
  }
}

void
Integrate(r32 dt_sec)
{

  assert(dt_sec > 0.f);
  // Delete any particles that must be deleted for this integration step.
  for (u32 i = 0; i < kUsedParticle2d;) {
    Particle2d* p = &kParticle2d[i];
    if (!FLAGGED(p->flags, kParticleRemove)) {
      ++i;
      continue;
    }

    BPDeleteP2d(p, i);
  }

  // Move all particles according to our laws of physics ignoring collisions.
  for (u32 i = 0; i < kUsedParticle2d; ++i) {
    Particle2d* p = &kParticle2d[i];
    if (FLAGGED(p->flags, kParticleFreeze)) continue;
    // Infinite mass object - do nothing.
    if (p->inverse_mass <= 0.f) continue;
    for (u32 j = 0; j < kUsedForceGenerator; ++j) {
      ForceGenerator* fg = &kForceGenerator[j];
      // TODO: Perhaps add proximity checks?
      fg->apply_force(p);
    }
    // F = m * a
    // a = F * (1 / m)
    v2f acc = p->acceleration;
    p->acceleration += p->force * p->inverse_mass;
    p->position += p->velocity * dt_sec;
    p->velocity += p->acceleration * dt_sec;
    p->velocity *= pow(p->damping, dt_sec);
    // Acceleration applied from forces only last a single frame.
    // Reverting here allows any user imposed acceleration to stick around.
    p->acceleration = acc;
    // Force applied over a single integration step then reset.
    p->force = {};

    BPUpdateP2d(p);
  }

  BPCalculateCollisions();
}

}  // namesapce physics
