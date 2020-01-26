
#include <cstdint>

#include "entity.cc"

constexpr uint64_t kFtlFrameTime = 60;
constexpr uint64_t kFtlCost = 100;

// TODO (AN): Perhaps avoid a mutable Ship.
// Provide a mutable ftl struct with const ship*
// Return true when a jump occurred... for now
bool
FtlUpdate(Ship* ship, uint64_t jump_frame)
{
  if (!jump_frame) {
    // No Jump
  } else if (jump_frame >= kFtlFrameTime) {
    // Jump
    ship->mineral -= kFtlCost;
    ship->level += 1;
    return true;
  }

  return false;
}
