#include <cstdint>
#include "synchapi.h"

namespace platform
{
s32
SleepSec(u64 sec)
{
  Sleep(sec * 1000);
  return 1;
}

s32
SleepMs(u64 duration)
{
  Sleep(duration);
  return 1;
}

s32
SleepUsec(u64 usec)
{
  SleepMs(usec / 1000);
  return 1;
}
}  // namespace platform
