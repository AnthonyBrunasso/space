#pragma once


#include "common/common.cc"
#include "math/vec.h"
#include "math/rect.h"

namespace physics {

struct Particle2d {
  u32 id;
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
  // Set to true when the particle is colliding with a 0.0 inverse mass
  // particle underneath it.
  b8 on_ground = false;

  // Particles keep forward and backward pointers so they can quickly sort
  // themselves after updating.
  u32 next_p2d_x = kInvalidId;
  u32 prev_p2d_x = kInvalidId;

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
  // Acceleration of gravity.
  r32 gravity = 100.f;
  // Linked list in sorted order on x / y axis for collision checks.
  u32 p2d_head_x = kInvalidId;
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

DECLARE_HASH_ARRAY(Particle2d, PHYSICS_PARTICLE_COUNT);

typedef void ApplyForceCallback(Particle2d* p);

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

Particle2d*
CreateInfinteMassParticle2d(v2f pos, v2f dims)
{
  Particle2d* particle = UseParticle2d();
  particle->position = pos;
  particle->dims = dims;
  particle->inverse_mass = 0.f;
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
__ResolvePositionAndVelocity(Particle2d* p, v2f correction)
{
  if (p->inverse_mass < FLT_EPSILON) return;
  if (IsZero(p->velocity)) return;
  if (correction.x > 0.f) {
    if (p->velocity.x > 0.f) {
      p->position.x -= correction.x;
    } else {
      p->position.x += correction.x;
    }
    p->velocity.x = 0.f;
  }
  if (correction.y > 0.f) {
    if (p->velocity.y > 0.f) {
      p->position.y -= correction.y;
    } else {
      p->position.y += correction.y;
    }
    p->velocity.y = 0.f;
  }
}

void
__SetOnGround(Particle2d* p1, Particle2d* p2, const Rectf& intersection)
{
  Rectf underneath_p1(p1->aabb());
  underneath_p1.y -= 1.f;
  underneath_p1.height = 2.f;
  underneath_p1.x += 1.f;
  underneath_p1.width -= 2.f;
  if (!p1->on_ground) {
    p1->on_ground = p2->inverse_mass < FLT_EPSILON &&
        math::IntersectRect(underneath_p1, intersection);
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
    
    // Set after collision code runs.
    p->on_ground = false;
    // F = m * a
    // a = F * (1 / m)
    v2f acc = p->acceleration;
    p->acceleration += p->force * p->inverse_mass;
    if (!FLAGGED(p->flags, kParticleIgnoreGravity)) {
      p->acceleration -= v2f(0.f, kPhysics.gravity);
    }
    p->velocity += p->acceleration * dt_sec;
    p->position += p->velocity * dt_sec;
    p->velocity *= pow(p->damping, dt_sec);
    // Acceleration applied from forces only last a single frame.
    // Reverting here allows any user imposed acceleration to stick around.
    p->acceleration = acc;
    // Force applied over a single integration step then reset.
    p->force = {};

    BPUpdateP2d(p);
  }

  BPCalculateCollisions();

  // Resolve collisions.
  for (u32 i = 0; i < kUsedBP2dCollision; ++i) {
    BP2dCollision* c = &kBP2dCollision[i];
    // Another correction may have moved this collision out of intersection.
    if (!math::IntersectRect(c->p1->aabb(), c->p2->aabb(), &c->intersection)) {
      continue;
    }

    __SetOnGround(c->p1, c->p2, c->intersection);
    __SetOnGround(c->p2, c->p1, c->intersection);
    
    // Use min axis of intersection to correct collisions.
    v2f correction;
    if (c->intersection.width < c->intersection.height) {
      correction = v2f(c->intersection.width, 0.f);
    } else {
      correction = v2f(0.f, c->intersection.height);
    }

    __ResolvePositionAndVelocity(c->p1, correction);
    __ResolvePositionAndVelocity(c->p2, correction);

    BPUpdateP2d(c->p1);
    BPUpdateP2d(c->p2);
  }
}

}  // namesapce physics
