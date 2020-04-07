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
  if (unit->player_index != player_index) return;
  unit->control |= (1 << player_index);
}

void
SelectPlayerModule(uint64_t player_index, Module* module)
{
  if (!module) return;
  if (module->player_index != player_index) return;
  module->control |= (1 << player_index);
}

uint64_t
CountUnitSelection(uint64_t player_index)
{
  unsigned player_control = (1 << player_index);
  uint64_t selection_count = 0;
  for (int i = 0; i < kUsedEntity; ++i) {
    if (0 == (kEntity[i].control & player_control)) continue;
    ++selection_count;
  }

  return selection_count;
}

void
UnselectPlayerAll(uint64_t player_index)
{
  unsigned player_control = (1 << player_index);
  for (int i = 0; i < kUsedEntity; ++i) {
    kEntity[i].control = ANDN(player_control, kEntity[i].control);
  }
}

}  // namespace simulation
