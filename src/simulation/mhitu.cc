// Missile Hit You

#include <cstdint>

#include "entity.cc"
#include "search.cc"
#include "tilemap.cc"

namespace simulation
{
uint64_t
MissileHitSimulation(Ship* ship, Missile* mi)
{
  uint64_t hit_frame = ship->frame - mi->hit_frame;
  const bool laser_defense = ship->crew_think_flags & FLAG(kUnitAiTurret);

  if (laser_defense) return 0;

  const v2i hit = mi->tile_hit;
  if (hit_frame % 5 == 0) {
    uint64_t replaced = BfsReplace(hit, 1, kTileOpen, kTileVacuum);

    LOGFMT("%ld mi impact %d %d replaced %lu tiles", hit_frame, hit.x, hit.y,
           replaced);
  }

  return 40 - hit_frame;
}

}  // namespace simulation
