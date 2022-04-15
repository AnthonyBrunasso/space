#pragma once

namespace platform {

// Exact implementation dependent on platform.
// See definitions in win32_clock / unix_clock.
struct Clock;

// Sets the start time for the clock.
void ClockStart(Clock* clock);

// Sets the end time for the clock and returns the elapsed microseconds.
u64 ClockEnd(Clock* clock);

// Returns the clocks elapsed microseconds.
u64 ClockDeltaUsec(const Clock& clock);

static r32 ClockDeltaSec(const Clock& clock) {
  u64 delta_usec = ClockDeltaUsec(clock);
  return SECONDS_R32(delta_usec);
}

}

#ifdef _WIN32
#include "win32_clock.cc"
#else
#include "unix_clock.cc"
#endif
