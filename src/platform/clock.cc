#pragma once

#ifndef _WIN32
#include <time.h>
#endif

namespace platform {

struct Clock {
#ifdef _WIN32
  // Start tick - Result of QueryPerformanceCounter
  uint64_t start_tick = 0;
  // End tick - Result of QueryPerformanceCounter
  uint64_t end_tick = 0;
#else
  // Start tick - Result of clock_gettime(CLOCK_MONOTONIC)
  struct timespec start;
  // End tick - Result of clock_gettime(CLOCK_MONOTONIC)
  struct timespec end;
#endif
};

void
ClockStart(Clock* clock);

// Returns the number of nanoseconds that have surpassed since ClockStart.
uint64_t
ClockEnd(Clock* clock);

uint64_t
ClockDeltaNsec(const Clock& clock);

}

#ifdef _WIN32
#include "win32_clock.cc"
#else
#include "unix_clock.cc"
#endif
