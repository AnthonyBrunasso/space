#include <cstdint>
#include "synchapi.h"

namespace platform {

s32 SleepSec(u64 sec) {
  Sleep((DWORD)(sec * 1000));
  return 1;
}

s32 SleepMs(u64 duration) {
  Sleep((DWORD)duration);
  return 1;
}

s32 SleepUsec(u64 usec) {
  SleepMs((DWORD)(usec / 1000));
  return 1;
}

}  // namespace platform
