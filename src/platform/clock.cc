#pragma once

#ifndef _WIN32
#include <time.h>
#endif

namespace platform {

// Exact implementation dependent on platform.
// See definitions in win32_clock / unix_clock.
struct Clock;

// Sets the start time for the clock.
void
ClockStart(Clock* clock);

// Sets the end time for the clock and returns the elapsed microseconds.
uint64_t
ClockEnd(Clock* clock);

// Returns the clocks elapsed microseconds.
uint64_t
ClockDeltaUsec(const Clock& clock);

}

#ifdef _WIN32
#include "win32_clock.cc"
#else
#include "unix_clock.cc"
#endif
