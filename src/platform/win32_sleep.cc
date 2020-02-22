#include <cstdint>
#include "synchapi.h"

namespace platform
{
void
_sleep_ms(uint64_t duration)
{
  Sleep(duration);
}
void
sleep_usec(uint64_t usec)
{
  _sleep_ms(usec / 1000);
}
}  // namespace platform
