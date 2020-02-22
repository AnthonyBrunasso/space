#include <cassert>
#include <cstdint>
#include <ctime>

namespace platform
{
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
