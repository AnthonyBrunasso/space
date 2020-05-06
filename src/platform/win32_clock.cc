#pragma once

#include <windows.h>
#include <profileapi.h>

namespace platform
{

struct Clock {
  // Start tick - Result of QueryPerformanceCounter
  uint64_t start_tick = 0;
  // End tick - Result of QueryPerformanceCounter
  uint64_t end_tick = 0;
};

static volatile uint64_t kClockFrequency = 0;

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

uint64_t
ClockEnd(Clock* clock)
{
  LARGE_INTEGER now;
  if (!QueryPerformanceCounter(&now)) {
    printf("Issue querying performance counter\n");
  }
  clock->end_tick = now.QuadPart;
  return ClockDeltaUsec(*clock);
}

uint64_t
ClockDeltaUsec(const Clock& clock)
{
  uint64_t elapsed_nano = (clock.end_tick - clock.start_tick) * 1e6;
  return elapsed_nano / kClockFrequency;
}

}
