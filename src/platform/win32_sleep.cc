#include <cstdint>
#include "synchapi.h"

namespace platform
{
int
sleep_sec(uint64_t sec)
{
  Sleep(sec * 1000);
  return 1;
}

int
sleep_ms(uint64_t duration)
{
  Sleep(duration);
  return 1;
}

int
sleep_usec(uint64_t usec)
{
  sleep_ms(usec / 1000);
  return 1;
}
}  // namespace platform
