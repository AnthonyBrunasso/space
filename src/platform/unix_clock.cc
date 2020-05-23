#pragma once

#include <cstdio>
#include <time.h>

namespace platform
{

struct Clock {
  // Start tick - Result of clock_gettime(CLOCK_MONOTONIC)
  struct timespec start;
  // End tick - Result of clock_gettime(CLOCK_MONOTONIC)
  struct timespec end;
};

void
ClockStart(Clock* clock)
{
  struct timespec t;
  if(clock_gettime(CLOCK_MONOTONIC, &clock->start) == -1) {
    printf("Error ClockStart\n");
  }
}

u64
ClockDeltaUsec(const Clock& clock)
{
  return (1000000000L * (clock.end.tv_sec - clock.start.tv_sec) +
      clock.end.tv_nsec - clock.start.tv_nsec) / 1e3;
}

u64
ClockEnd(Clock* clock)
{
  struct timespec t;
  if(clock_gettime(CLOCK_MONOTONIC, &clock->end) == -1) {
    printf("Error ClockStart\n");
  }
  return ClockDeltaUsec(*clock);
}

}
