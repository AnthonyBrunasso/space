#pragma once

#include "platform/clock.cc"

namespace util {

struct Cooldown {
  u64 usec = 0; 
  platform::Clock clock;
};

void
CooldownReset(Cooldown* cooldown)
{
  platform::ClockStart(&cooldown->clock);
}

bool
CooldownReady(Cooldown* cooldown)
{
  platform::ClockEnd(&cooldown->clock);
  return platform::ClockDeltaUsec(cooldown->clock) > cooldown->usec;
}

}
