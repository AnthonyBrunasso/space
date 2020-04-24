#include <cassert>
#include <cstdint>
#include <ctime>

namespace platform
{
int
sleep_sec(uint64_t sec)
{
  struct timespec duration;
  duration.tv_sec = sec;
  duration.tv_nsec = 0;
  return nanosleep(&duration, NULL);
}

int
sleep_ms(uint64_t msec)
{
  struct timespec duration;
  duration.tv_sec = msec / 1000;
  duration.tv_nsec = (msec % 1000) * 1e6;
  return nanosleep(&duration, NULL);
}

// Return -1 if interrupted
// Return 0 on completed sleep
int
sleep_usec(uint64_t usec)
{
  assert(usec < 2 * (1000 * 1000));

  struct timespec duration;
  duration.tv_sec = 0;
  duration.tv_nsec = usec * 1000;
  return nanosleep(&duration, NULL);
}

}  // namespace platform
