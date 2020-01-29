#pragma once

#include <cstdint>

#include "platform/platform.cc"

#include "entity.cc"

namespace simulation
{
constexpr uint64_t kFtlFrameTime = 60;
constexpr uint64_t kFtlCost = 100;

void
FtlInit(FtlState* ftl)
{
  constexpr uint64_t not_flags = FLAG(kFtlTangible);
  ftl->state_flags = ANDN(not_flags, ftl->state_flags);
}

// Return true when a jump occurred
bool
FtlUpdate(const Ship* ship, FtlState* ftl)
{
  // When ftl_frame ceases to advance, a jump is processing
  ftl->frame += ftl->state_flags & FLAG(kFtlTangible);
  uint64_t frame_offset = ship->frame - ftl->frame;
  if (!frame_offset) {
    // No Jump
  } else if (frame_offset >= kFtlFrameTime) {
    for (int i = 0; i < kUsedMissile; ++i) {
      kMissile[i] = kZeroMissile;
    }
    for (int i = 0; i < kUsedAsteroid; ++i) {
      kAsteroid[i] = kZeroAsteroid;
    }

    // ftl becomes tangible again after a jump completes
    ftl->state_flags |= FLAG(kFtlTangible);
    ftl->frame += kFtlFrameTime;
    return true;
  }

  return false;
}

}  // namespace simulation
