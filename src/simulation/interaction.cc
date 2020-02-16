
#include "entity.cc"
#include "ftl.cc"
#include "tilemap.cc"

namespace simulation
{
constexpr float kDsqSelect = 25.f * 25.f;

bool
ShipFtlReady()
{
  return kShip[0].sys_engine > .5f && kShip[0].mineral >= kFtlCost;
}

void
ControlShipFtl()
{
  kShip[0].ftl_frame = 1;
}

uint64_t
SelectUnit(v3f world)
{
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];

    if (dsq(unit->transform.position, world) < kDsqSelect) {
      return i;
    }
  }

  return kMaxUnit;
}

void
ControlUnit(uint64_t unit_id)
{
  for (int i = 0; i < kUsedUnit; ++i) {
    bool is_unit = i == unit_id;
    kUnit[i].kind = !is_unit * (i + 1);
  }
}

}  // namespace simulation
