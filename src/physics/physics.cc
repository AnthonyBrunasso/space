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

struct Particle2dCollision {
  Particle2d* p1;
  Particle2d* p2;
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
DECLARE_ARRAY(Particle2dCollision, PHYSICS_PARTICLE_COUNT);

typedef void ApplyForceCallback(Particle2d* p);

struct ForceGenerator {
  ApplyForceCallback* apply_force = nullptr;
};

DECLARE_ARRAY(ForceGenerator, 8);

// Sorted insertion into a double linked list.
#define INSERT_SORTED(p, a)                                             \
  if (!kPhysics.p2d_head_##a) {                                         \
    kPhysics.p2d_head_##a = particle;                                   \
  } else {                                                              \
    if (particle->position.a < kPhysics.p2d_head_##a->position.a) {     \
      kPhysics.p2d_head_##a->prev_p2d_##a = particle;                   \
      particle->next_p2d_##a = kPhysics.p2d_head_##a;                   \
      kPhysics.p2d_head_##a = particle;                                 \
    } else {                                                            \
      Particle2d* prev = kPhysics.p2d_head_##a;                         \
      Particle2d* next = kPhysics.p2d_head_##a->next_p2d_##a;           \
      while (prev) {                                                    \
        if (!next) {                                                    \
          particle->prev_p2d_##a = prev;                                \
          prev->next_p2d_##a = particle;                                \
          break;                                                        \
        }                                                               \
        if (particle->position.a >= prev->position.a &&                 \
            particle->position.a <= next->position.a) {                 \
          particle->prev_p2d_##a = prev;                                \
          particle->next_p2d_##a = next;                                \
          next->prev_p2d_##a = particle;                                \
          prev->next_p2d_##a = particle;                                \
          break;                                                        \
        }                                                               \
        prev = next;                                                    \
        next = next->next_p2d_##a;                                      \
      }                                                                 \
    }                                                                   \
  }

Particle2d*
CreateParticle2d(v2f pos, v2f dims)
{
  Particle2d* particle = UseParticle2d();
  particle->position = pos;
  particle->dims = dims;
  INSERT_SORTED(particle, x);
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

// Moves particle to between prev and next pointers.
void
__MoveP2d(Particle2d* p, Particle2d* prev, Particle2d* next)
{
  // Consider the case where head must be updated.
  // If p was head it is being moved forward. Its next is now head.
  if (p == kPhysics.p2d_head_x) {
    kPhysics.p2d_head_x = p->next_p2d_x;
  }
  // If p has the smallest x in the list it will become the new head.
  if (p->position.x < kPhysics.p2d_head_x->position.x) {
    kPhysics.p2d_head_x = p;
  }
  // Swap all node pointers
  if (p->prev_p2d_x) p->prev_p2d_x->next_p2d_x = p->next_p2d_x;
  if (p->next_p2d_x) p->next_p2d_x->prev_p2d_x = p->prev_p2d_x;
  if (prev) prev->next_p2d_x = p;
  if (next) next->prev_p2d_x = p;
  p->next_p2d_x = next;
  p->prev_p2d_x = prev;
}

// Updates the particle to be sorted.
void
__SortP2d(Particle2d* p)
{
  // Updates particle position in sorted list if its x is now out of order.
  Particle2d* prev = p->prev_p2d_x;
  Particle2d* next = p->next_p2d_x;
  // Move the particle forward if needed.
  while (next && p->position.x > next->position.x) {
    prev = next;
    next = next->next_p2d_x;
  }
  if (next != p->next_p2d_x) {
    __MoveP2d(p, prev, next);
  }
  prev = p->prev_p2d_x;
  next = p->next_p2d_x;
  // Move the particle backward if needed.
  while (prev && p->position.x < prev->position.x) {
    next = prev;
    prev = prev->prev_p2d_x;
  }
  if (prev != p->prev_p2d_x) {
    __MoveP2d(p, prev, next);
  }
}

void
Integrate(r32 dt_sec)
{
  kUsedParticle2dCollision = 0;

  assert(dt_sec > 0.f);
  // Delete any particles that must be deleted for this integration step.
  for (u32 i = 0; i < kUsedParticle2d;) {
    Particle2d* p = &kParticle2d[i];
    if (!FLAGGED(p->flags, kParticleRemove)) {
      ++i;
      continue;
    }

    // If we are deleting the head adjust the head pointer.
    if (p == kPhysics.p2d_head_x) {
      kPhysics.p2d_head_x = kPhysics.p2d_head_x->next_p2d_x;
    }

    // Get the particles next and prev pointers - update their next and prev.
    Particle2d* nextx = p->next_p2d_x; 
    Particle2d* prevx = p->prev_p2d_x; 

    if (nextx) {
      if (prevx) nextx->prev_p2d_x = prevx;
      else nextx->prev_p2d_x = nullptr;
    }

    if (prevx) {
      if (nextx) prevx->next_p2d_x = nextx;
      else prevx->next_p2d_x = nullptr;
    }

    // Swap the last and and current particle of the used list.
    kParticle2d[i] = kParticle2d[kUsedParticle2d - 1];
    kParticle2d[kUsedParticle2d - 1] = {};

    // This is the case that the fixed pointers actually point to an index
    // that gets swapped.
    if (nextx && nextx->prev_p2d_x &&
        nextx->prev_p2d_x == &kParticle2d[kUsedParticle2d - 1]) {
      nextx->prev_p2d_x = &kParticle2d[i];
    }

    if (prevx && prevx->next_p2d_x &&
        prevx->next_p2d_x == &kParticle2d[kUsedParticle2d - 1]) {
      prevx->next_p2d_x = &kParticle2d[i];
    }

    if (kParticle2d[i].prev_p2d_x) {
      kParticle2d[i].prev_p2d_x->next_p2d_x = &kParticle2d[i];
    }

    --kUsedParticle2d;

    if (!kUsedParticle2d) {
      kPhysics.p2d_head_x = nullptr;
    }
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

    __SortP2d(p);
  }

  // Detect all colliding particles.
  Particle2d* p1 = kPhysics.p2d_head_x;
  Particle2d* p2 = p1->next_p2d_x;
  while (p1->next_p2d_x) {
    if (!p2) {
      p1 = p1->next_p2d_x;
      p2 = p1->next_p2d_x;
      continue;
    }
    if (math::IntersectRect(p1->aabb(), p2->aabb())) {
      Particle2dCollision* collision = UseParticle2dCollision();
      collision->p1 = p1;
      collision->p2 = p2;
      p2 = p2->next_p2d_x;
      continue;
    }
    // If x axis intersects move p2 forward only, this may be a non
    // intersecting y.
    if (p2->aabb().Min().x < p1->aabb().Max().x) {
      p2 = p2->next_p2d_x;
    } else {
      p1 = p1->next_p2d_x;
      p2 = p1->next_p2d_x;
    }
  }
}

}  // namesapce physics
