#pragma once

#include "math/vec.h"

#include "entity.cc"
#include "tilemap.cc"

#include <cstdint>

namespace simulation
{
constexpr float kDsqSelect = 25.f * 25.f;

int
AssignPlayerId()
{
  static int id = 0;
  return (id++ % kPlayerCount);
}

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

Module*
GetModule(const math::Rectf& rect, int idx)
{
  Module* mod = &kModule[idx];
  TilemapModify tm(mod->ship_index);
  // TODO: Take into consideration module bounds and do a rect/rect intersect.
  //v2f t = TilePosToWorld(mod->tile);
  //printf("GetModule: %.2f,%.2f tile: %i,%i\n",
  //       t.x, t.y, mod->tile.x, mod->tile.y);
  if (PointInRect(TilePosToWorld(mod->tile), rect)) {
    return mod;
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

uint32_t
ScenarioSpawnEnemy(v2i tile_position, uint64_t ship_index)
{
  // Uses raii to revert ship index back to whatever was set.
  TilemapModify tm(ship_index);
  Unit* enemy = UseIdUnit();
  enemy->ship_index = ship_index;
  enemy->transform.position = TilePosToWorld(tile_position);
  enemy->transform.scale = v3f(0.25f, 0.25f, 0.f);
  enemy->alliance = kEnemy;
  enemy->kind = kAlien;
  enemy->attack_radius = 30.f;
  enemy->speed = 0.5f;
  BB_SET(enemy->bb, kUnitBehavior, kUnitBehaviorAttackWhenDiscovered);
  return enemy->id;
}

void
ScenarioSpawnCrew(v3f world_position, uint64_t ship_index)
{
  TilemapModify tm(ship_index);
  Unit* unit = UseIdUnit();
  unit->ship_index = ship_index;
  unit->transform.position = world_position;
  unit->transform.scale = v3f(0.25f, 0.25f, 0.f);
  unit->kind = kOperator;
  unit->spacesuit = 1;
  unit->speed = 1.0f;
  unit->player_id = AssignPlayerId();
}

void
ScenarioSpawnCrew(v2i tile_position, uint64_t ship_index)
{
  TilemapModify tm(ship_index);
  Unit* unit = UseIdUnit();
  unit->ship_index = ship_index;
  unit->transform.position = TilePosToWorld(tile_position);
  unit->transform.scale = v3f(0.25f, 0.25f, 0.f);
  unit->kind = kOperator;
  unit->spacesuit = 1;
  unit->speed = 1.0f;
  unit->player_id = AssignPlayerId();
}

}  // namespace simulation
