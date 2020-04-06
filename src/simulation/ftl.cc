#pragma once

#include <cstdint>

#include "platform/platform.cc"

#include "entity.cc"

namespace simulation
{
constexpr uint64_t kFtlFrameTime = 60;
constexpr uint64_t kFtlCost = 100;

uint64_t
FtlSimulation(const Ship* ship)
{
  const uint64_t ftl_frame = ship->ftl_frame;
  if (ftl_frame == kFtlFrameTime) {
    for (int i = 0; i < kUsedAsteroid; ++i) {
      kAsteroid[i] = kZeroAsteroid;
    }
  }

  return kFtlFrameTime - ftl_frame;
}

}  // namespace simulation
