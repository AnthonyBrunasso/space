#pragma once

namespace simulation
{

void
AIWander(Unit* unit)
{
  // Non interrupting behavior.
  if (unit->uaction != kUaNone) return;
  v2i rpos = TileRandomNeighbor(WorldToTilePos(unit->transform.position));
  v2f wpos = TilePosToWorld(rpos);
  BB_SET(unit->bb, kUnitDestination, v3f(wpos.x, wpos.y, 0.f));
  unit->uaction = kUaMove;
}

void
AISimpleBehavior(Unit* unit)
{
  // Non interrupting behavior.
  if (unit->uaction != kUaNone) return;
  AIWander(unit);
}

void
AIThink(Unit* unit)
{
  if (!unit) return;
  const int* behavior;
  if (!BB_GET(unit->bb, kUnitBehavior, behavior)) return;
  switch (*behavior) {
    case kUnitBehaviorSimple: {
      AISimpleBehavior(unit);
    } break;
    default: break;
  }
}

}  // namespace simulation
