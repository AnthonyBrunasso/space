#include <cstdint>
#include "synchapi.h"

namespace platform
{
int
SleepSec(uint64_t sec)
{
  Sleep(sec * 1000);
  return 1;
}

int
SleepMs(uint64_t duration)
{
  Sleep(duration);
  return 1;
}

int
SleepUsec(uint64_t usec)
{
  SleepMs(usec / 1000);
  return 1;
}
}  // namespace platform
