#pragma once

#include <cstdio>
#include <windows.h>
#include <profileapi.h>

namespace platform
{

struct Clock {
  // Start tick - Result of QueryPerformanceCounter
  u64 start_tick = 0;
  // End tick - Result of QueryPerformanceCounter
  u64 end_tick = 0;
};

static volatile u64 kClockFrequency = 0;

void
ClockStart(Clock* clock)
{
  if (!kClockFrequency) {
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq)) {
      printf("Issue querying performance frequency\n");
    }
    kClockFrequency = freq.QuadPart;
  }

  LARGE_INTEGER now;
  if (!QueryPerformanceCounter(&now)) {
    printf("Issue querying performance counter\n");
  }
  clock->start_tick = now.QuadPart;
}

u64
ClockEnd(Clock* clock)
{
  LARGE_INTEGER now;
  if (!QueryPerformanceCounter(&now)) {
    printf("Issue querying performance counter\n");
  }
  clock->end_tick = now.QuadPart;
  return ClockDeltaUsec(*clock);
}

u64
ClockDeltaUsec(const Clock& clock)
{
  u64 elapsed = (clock.end_tick - clock.start_tick) * 1e6;
  return elapsed / kClockFrequency;
}

}
