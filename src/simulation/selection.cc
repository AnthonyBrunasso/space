#pragma once

#include "math/math.cc"

#include "entity.cc"

namespace simulation
{
struct SelectionBox {
  v2f start;
  math::Rectf rect;
};

// Store both to avoid needing to calculate scale between screen and world.
static SelectionBox kSelectionBoxScreen;
static SelectionBox kSelectionBoxWorld;
static bool kRenderSelectionBox = false;

void
UpdateBoxSelectionOnPosition(v2f screen, v3f world)
{
  v2f sd = screen - kSelectionBoxScreen.start;
  kSelectionBoxScreen.rect = math::Rectf(
      kSelectionBoxScreen.start.x, kSelectionBoxScreen.start.y, sd.x, sd.y);
  v2f wd = world.xy() - kSelectionBoxWorld.start;
  kSelectionBoxWorld.rect = math::Rectf(kSelectionBoxWorld.start.x,
                                       kSelectionBoxWorld.start.y, wd.x, wd.y);
}

void
UpdateBoxSelectionOnClick(v2f screen, v3f world)
{
  kSelectionBoxScreen.start = screen;
  kSelectionBoxWorld.start = world.xy();
  kSelectionBoxScreen.rect = math::Rectf(screen.x, screen.y, 0.0f, 0.0f);
  kSelectionBoxWorld.rect = math::Rectf(world.x, world.y, 0.0f, 0.0f);
  kRenderSelectionBox = true;
}

void
SelectUnit(uint32_t unit_id)
{
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
