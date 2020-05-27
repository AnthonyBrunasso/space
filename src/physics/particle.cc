#pragma once

#include "common/common.cc"
#include "math/vec.h"
#include "math/rect.h"

namespace physics {

struct Particle2d {
  v2f position;       // Position of particle - center of aabb.
  v2f velocity;
  v2f acceleration;
  r32 damping = 1.f; // Damping force applied to particle to slow it down.
  v2f dims;          // Used to create the aabb.

  Rectf aabb() const {
    return Rectf(position - dims / 2.f, dims);
  }
};

DECLARE_ARRAY(Particle2d, PARTICLE_COUNT);

}  // namesapce physics
