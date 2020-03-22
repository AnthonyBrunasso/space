#pragma once

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
  return TilePosToWorld(mod->tile);
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

Unit*
GetUnit(v3f world)
{
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    if (v3fDsq(unit->transform.position, world) < kDsqSelect) {
      return unit;
    }
  }

  return nullptr;
}

Unit*
GetUnitTarget(uint64_t local_player, v3f world) {
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    if (FLAGGED(unit->control, local_player)) continue;
    if (v3fDsq(unit->transform.position, world) < kDsqSelect) {
      return unit;
    }
  }

  return nullptr;
}

Unit*
GetUnit(const math::Rectf& rect, int idx)
{
  Unit* unit = &kUnit[idx];
  if (PointInRect(unit->transform.position.xy(), rect)) {
    return unit;
  }
  return nullptr;
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
InRange(uint64_t unit_id, uint64_t target_id)
{
  if (unit_id == kInvalidUnit || target_id == kInvalidUnit) return false;
  Unit* source_unit = FindUnit(unit_id);
  Unit* target_unit = FindUnit(target_id);
  return InRange(source_unit, target_unit);
}

bool
ShouldAttack(Unit* unit, Unit* target)
{
  if (!unit || !target) return false;
  if (unit->alliance == target->alliance) return false;
  return true;
}

bool
ShouldAttack(uint64_t unit, uint64_t target)
{
  if (unit == kInvalidUnit || target == kInvalidUnit) return false;
  return ShouldAttack(FindUnit(unit), FindUnit(target));
}

Unit*
FindUnitInRangeToAttack(Unit* unit)
{
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* target = &kUnit[i];
    if (unit == target) continue;
    if (!ShouldAttack(unit, target)) continue;
    if (InRange(unit, target)) return target;
  }
  return nullptr;
}

Unit*
GetNearestEnemyUnit(Unit* unit)
{
  float d = FLT_MAX;
  Unit* target = nullptr;
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* new_target = &kUnit[i];
    float nd = v3fDsq(unit->transform.position,
                      new_target->transform.position);
    if (nd < d && ShouldAttack(unit, new_target)) {
      target = new_target;
      d = nd;
    }
  }
  return target;
}

}  // namespace simulation
