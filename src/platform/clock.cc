#pragma once

namespace platform {

struct Clock {
#ifdef _WIN32
  uint64_t frequency;
#endif
  // The ideal advacement cadence of tsc_clock
  uint64_t tsc_step;
  // Count the time rhythmic progression was lost
  uint64_t jerk;
  // Rhythmic time progression
  uint64_t tsc_clock;
  // Used for time delta within a single frame
  uint64_t frame_to_frame_tsc;
};

Clock
ClockCreate(uint64_t frame_goal_usec);

bool
ClockSync(Clock* clock, uint64_t* optional_sleep_usec);

uint64_t
ClockDeltaUsec(const Clock& clock);

uint64_t
TscDeltaToUsec(const Clock& clock, uint64_t delta_tsc);

bool
ClockSync(uint64_t tsc_now, Clock* clock, uint64_t* optional_sleep_usec)
{
  uint64_t tsc_previous = clock->tsc_clock;
  uint64_t tsc_next = tsc_previous + clock->tsc_step;

  if (tsc_next - tsc_now < clock->tsc_step) {
    // no-op, busy wait
    // optional sleep time
    *optional_sleep_usec = TscDeltaToUsec(*clock, tsc_next - tsc_now);
    return false;
  } else if (tsc_now - tsc_next <= clock->tsc_step) {
    // frame slightly over goal time
    // trivial advancement continues
    // within tsc_step do not sleep
    clock->tsc_clock = tsc_next;
  } else if (tsc_now - tsc_next > clock->tsc_step) {
    // non-trivial time-jerk, due to hardware clock or massive perf issue
    // do not sleep
    clock->tsc_clock = tsc_now;
    clock->jerk += 1;
  }

  clock->frame_to_frame_tsc = tsc_now;
  return true;
}

}

#ifdef _WIN32
#include "win32_clock.cc"
#else
#include "unix_clock.cc"
#endif
