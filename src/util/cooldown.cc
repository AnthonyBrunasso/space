#pragma once

#include "platform/clock.cc"

namespace util {

struct Cooldown {
  u64 usec = 0; 
  platform::Clock clock;
  // Allows cooldown to initialize to ready.
  b8 overwrite_ready = false;
};

void
CooldownReset(Cooldown* cooldown)
{
  platform::ClockStart(&cooldown->clock);
  cooldown->overwrite_ready = false;
}

void
CooldownInitialize(Cooldown* cooldown)
{
  CooldownReset(cooldown);
  cooldown->overwrite_ready = true;
}

bool
CooldownReady(Cooldown* cooldown)
{
  platform::ClockEnd(&cooldown->clock);
  if (cooldown->overwrite_ready) return true;
  return platform::ClockDeltaUsec(cooldown->clock) > cooldown->usec;
}

}
