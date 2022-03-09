#pragma once

#include "platform/clock.cc"

namespace util {

struct Cooldown {
  u64 usec = 0; 
  platform::Clock clock;
  // Allows cooldown to initialize to ready.
  b8 overwrite_ready = false;
};

void CooldownReset(Cooldown* cooldown) {
  platform::ClockStart(&cooldown->clock);
  cooldown->overwrite_ready = false;
}

void CooldownInitialize(Cooldown* cooldown) {
  CooldownReset(cooldown);
  cooldown->overwrite_ready = true;
}

bool CooldownReady(Cooldown* cooldown) {
  platform::ClockEnd(&cooldown->clock);
  if (cooldown->overwrite_ready) return true;
  return platform::ClockDeltaUsec(cooldown->clock) > cooldown->usec;
}

struct FrameCooldown {
  // Number of frames allowed before FrameCooldownReady returns true.
  u64 frame = 0;
  u64 frame_start = 0;
  b8 overwrite_ready = false;
};

static u64 kFrameNum = 0;

void FrameCooldownUpdate() {
  ++kFrameNum;
}

void FrameCooldownReset(FrameCooldown* cooldown) {
  cooldown->frame_start = kFrameNum;
  cooldown->overwrite_ready = false;
}

void FrameCooldownInitialize(FrameCooldown* cooldown) {
  FrameCooldownReset(cooldown);
  cooldown->overwrite_ready = true;
}

bool FrameCooldownReady(FrameCooldown* cooldown) {
  if (cooldown->overwrite_ready) return true;
  return kFrameNum - cooldown->frame_start > cooldown->frame;
}

}
