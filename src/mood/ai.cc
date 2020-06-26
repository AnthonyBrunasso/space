#pragma once

#include "mood/entity.cc"
#include "physics/physics.cc"

namespace mood {

struct AI {
  // Cooldown for when to spawn an enemy.
  util::Cooldown enemy_cooldown;
};

static AI kAI;

void
AICreate(v2f pos, v2f dims)
{
  UseEntityCharacter(pos, dims);
  // Initialize AI stuff?
}

void
AIInitialize()
{
  kAI.enemy_cooldown.usec = SECONDS(1.f);
  util::CooldownInitialize(&kAI.enemy_cooldown);
}

void
AIUpdate()
{
  if (util::CooldownReady(&kAI.enemy_cooldown)) {
    AICreate(
        v2f(math::ScaleRange((r32)rand() / RAND_MAX, 0.f, 1.f, 10.f, 100.f),
            10.f), v2f(5.f, 5.f));
    util::CooldownReset(&kAI.enemy_cooldown);
  }
}

}
