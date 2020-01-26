// Missile Hit You

#include <cstdint>

#include "entity.cc"
#include "search.cc"
#include "tilemap.cc"

namespace simulation
{
void
MissileHitShip(Ship* ship, Missile* mi)
{
  const v2i hit = mi->tile_hit;
  const bool laser_defense = ship->crew_think_flags & FLAG(kUnitAiTurret);

  if (laser_defense) return;

  uint64_t replaced = BfsReplace(hit, 8, kTileOpen, kTileVacuum);
  printf("mi impact %d %d replaced %lu tiles\n", hit.x, hit.y, replaced);
}

}  // namespace simulation
