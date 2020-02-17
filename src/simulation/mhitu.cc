// Missile Hit You

#include <cstdint>

#include "entity.cc"
#include "search.cc"
#include "tilemap.cc"

namespace simulation
{
constexpr uint64_t kMissileExplodeTime = 40;

uint64_t
MissileHitSimulation(Missile* mi)
{
  uint64_t explode_frame = mi->explode_frame;

  const v2i hit = mi->tile_hit;
  if (explode_frame % 5 == 1) {
    uint64_t replaced = BfsRemoveOxygen(hit, 1);

    LOGFMT("%ld mi impact %d %d replaced %lu tiles (disabled)", explode_frame,
           hit.x, hit.y, replaced);
  }

  return kMissileExplodeTime - explode_frame;
}

}  // namespace simulation
