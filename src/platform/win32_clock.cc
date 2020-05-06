#pragma once

#include <windows.h>
#include <profileapi.h>

namespace platform
{

Clock
ClockCreate(uint64_t frame_goal_usec)
{
  LARGE_INTEGER now, freq;
  if (!QueryPerformanceFrequency(&freq)) {
    printf("Issue querying performance frequency\n");
  }
  if (!QueryPerformanceCounter(&now)) {
    printf("Issue querying performance counter\n");
  }
  Clock clock;
  clock.frequency = freq.QuadPart;
  clock.tsc_step = (frame_goal_usec * clock.frequency) / 1e6;
  clock.jerk = 0;
  clock.tsc_clock = now.QuadPart;
  clock.frame_to_frame_tsc = now.QuadPart;
  return clock;
}


uint64_t
ClockDeltaUsec(const Clock& clock)
{
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  uint64_t elapsed_micro = now.QuadPart - clock.frame_to_frame_tsc;
  elapsed_micro *= 1e6;
  return elapsed_micro / clock.frequency;
}

uint64_t
TscDeltaToUsec(const Clock& clock, uint64_t delta_tsc)
{
  return (delta_tsc * 1e6) / clock.frequency;
}

bool
ClockSync(Clock* clock, uint64_t* optional_sleep_usec)
{
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  return ClockSync(now.QuadPart, clock, optional_sleep_usec);
}

}
