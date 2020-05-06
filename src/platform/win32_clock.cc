#pragma once

#include <windows.h>
#include <profileapi.h>

namespace platform
{

static volatile uint64_t kClockFrequency = 0;

void
ClockStart(Clock* clock)
{
  if (!kClockFrequency) {
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq)) {
      printf("Issue querying performance frequency\n");
      kClockFrequency = freq.QuadPart;
    }
  }

  LARGE_INTEGER now;
  if (!QueryPerformanceCounter(&now)) {
    printf("Issue querying performance counter\n");
  }
  clock->start = now.QuadPart;
}

uint64_t
ClockEnd(Clock* clock)
{
  LARGE_INTEGER now;
  if (!QueryPerformanceCounter(&now)) {
    printf("Issue querying performance counter\n");
  }
  clock->end = now.QuadPart;
  return ClockDeltaNsec(clock);
}

uint64_t
ClockDeltaNsec(const Clock& clock)
{
  uint64_t elapsed_nano = (clock.end - clock.start) * 1e9;
  return elapsed_nano / kClockFrequency;
}

}
