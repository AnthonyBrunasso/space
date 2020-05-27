#pragma once

#include "common/common.cc"
#include "math/vec.h"
#include "math/rect.h"

namespace physics {

struct Particle2d {
  v2f position;       // Position of particle - center of aabb.
  v2f velocity;
  v2f acceleration;
  // Inverse mass of this particle. Inverse infinite mass can be reprented
  // with a value of 0 and since 0 mass objects have no physical
  // representation.
  r32 inverse_mass = 1.f;
  r32 damping = 0.9f; // Damping force applied to particle to slow it down.
  v2f dims;           // Used to create the aabb.

  Rectf aabb() const {
    return Rectf(position - dims / 2.f, dims);
  }
};

DECLARE_ARRAY(Particle2d, PARTICLE_COUNT);

void
Integrate(r32 dt)
{
  assert(dt > 0.f);
  for (u32 i = 0; i < kUsedParticle2d; ++i) {
    Particle2d* p = &kParticle2d[i];
    // Infinite mass object - do nothing.
    if (p->inverse_mass <= 0.f) continue;
    p->position += p->velocity * dt;
    p->velocity += p->acceleration * dt;
    p->velocity *= pow(p->damping, dt);
  }
}

}  // namesapce physics
