
#include "math/vec.h"

#include "entity.cc"
#include "tilemap.cc"

#include <cstdint>

namespace simulation
{
constexpr float kDsqSelect = 25.f * 25.f;
v3f
v3fModule(Module* mod)
{
  return TilePosToWorld(v2i(mod->cx, mod->cy));
}

float
v3fDsq(const v3f& dst, const v3f& src)
{
  v3f delta = dst - src;
  return LengthSquared(delta);
}

// Call using GAME_ITER macro in entity.cc
uint64_t
v3fNearTransform(v3f pos, uint64_t step, const uint8_t* start,
                 const uint8_t* end, float* dsq)
{
  uint64_t index = UINT64_MAX;
  float nearest = FLT_MAX;
  int i = 0;
  for (const uint8_t* iter = start; iter < end; iter += step, i += 1) {
    const Transform* t = (const Transform*)iter;
    float distance = v3fDsq(pos, t->position);
    if (distance < nearest) {
      nearest = distance;
      index = i;
    }
  }

  *dsq = nearest;
  return index;
}

uint32_t
GetUnitId(v3f world)
{
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];

    if (v3fDsq(unit->transform.position, world) < kDsqSelect) {
      return unit->id;
    }
  }

  return kInvalidUnit;
}

bool
GetUnitId(const math::Rectf& rect, int idx, uint32_t* id)
{
  Unit* unit = &kUnit[idx];
  if (PointInRect(unit->transform.position.xy(), rect)) {
    *id = unit->id;
    return true;
  }
  return false;
}

bool
InRange(Unit* source_unit, Unit* target_unit)
{
  float dsq =
      v3fDsq(source_unit->transform.position, target_unit->transform.position);
  float rsq = source_unit->attack_radius * source_unit->attack_radius;
  return dsq < rsq;
}

bool
InRange(uint64_t unit, uint64_t target)
{
  if (unit == kInvalidUnit || target == kInvalidUnit) return false;
  Unit* source_unit = FindUnit(unit);
  Unit* target_unit = FindUnit(target);
  return InRange(source_unit, target_unit);
}

}  // namespace simulation
