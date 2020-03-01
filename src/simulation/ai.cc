#pragma once

namespace simulation
{

// AI Will wander to adjacent tiles.
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

// AI Will wander to adjacent tiles until it finds someone it can attack or
// a unit attacks it - in which case it will attack back.
void
AISimpleBehavior(Unit* unit)
{
  // Non interrupting behavior.
  if (unit->uaction != kUaNone) return;

  const uint32_t* attacker;
  if (BB_GET(unit->bb, kUnitAttacker, attacker)) {
    Unit* target = FindUnit(*attacker);
    if (target) {
      BB_SET(unit->bb, kUnitTarget, target->id);
      unit->uaction = kUaAttack;
      return;
    }
  }

  Unit* nearby_target = FindUnitInRangeToAttack(unit);
  if (nearby_target) {
    BB_SET(unit->bb, kUnitTarget, nearby_target->id);
    unit->uaction = kUaAttack;
    return;
  }

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

  // Clear AI knowledge that should be learned per tick.
  BB_REM(unit->bb, kUnitAttacker);
}

}  // namespace simulation