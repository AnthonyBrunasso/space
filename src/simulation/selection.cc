#pragma once

#include "math/math.cc"

#include "entity.cc"

namespace simulation
{
void
SelectPlayerUnit(uint64_t player_index, Unit* unit)
{
  if (!unit) return;
  if (unit->alliance == kEnemy) return;
  if (unit->kind == kAlien) return;
  if (unit->player_id != player_index) return;
  unit->control |= (1 << player_index);
}

void
SelectPlayerModule(uint64_t player_index, Module* module)
{
  if (!module) return;
  if (module->player_id != player_index) return;
  module->control |= (1 << player_index);
}

uint64_t
CountUnitSelection(uint64_t player_index)
{
  unsigned player_control = (1 << player_index);
  uint64_t selection_count = 0;
  for (int i = 0; i < kUsedUnit; ++i) {
    if (0 == (kUnit[i].control & player_control)) continue;
    ++selection_count;
  }

  return selection_count;
}

void
UnselectPlayerUnits(uint64_t player_index)
{
  unsigned player_control = (1 << player_index);
  for (int i = 0; i < kUsedUnit; ++i) {
    kUnit[i].control = ANDN(player_control, kUnit[i].control);
  }
}

}  // namespace simulation
