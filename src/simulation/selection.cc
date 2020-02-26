#pragma once

#include "math/math.cc"

#include "entity.cc"

namespace simulation
{
void
SelectUnit(uint32_t unit_id)
{
  if (unit_id == kInvalidUnit) return;
  Selection* selection = UseSelection();
  selection->unit_id = unit_id;
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
