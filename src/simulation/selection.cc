#pragma once

#include "math/math.cc"

#include "entity.cc"

namespace simulation
{
void
SelectUnit(Unit* unit)
{
  if (!unit) return;
  if (unit->alliance == kEnemy) return;
  if (unit->kind == kAlien) return;
  Selection* selection = UseSelection();
  selection->unit_id = unit->id;
}

bool
IsUnitSelected(uint32_t unit_id)
{
  for (int i = 0; i < kUsedSelection; ++i) {
    if (kSelection[i].unit_id == unit_id) {
      return true;
    }
  }
  return false;
}

void
UnselectAll()
{
  kUsedSelection = 0;
}

}  // namespace simulation
