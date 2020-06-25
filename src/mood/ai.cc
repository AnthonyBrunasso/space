#pragma once

#include "mood/entity.cc"
#include "physics/physics.cc"

namespace mood {

void
AICreate(v2f pos, v2f dims)
{
  UseEntityCharacter(pos, dims);
  // Initialize AI stuff?
}

void
AIUpdate()
{
}

}
