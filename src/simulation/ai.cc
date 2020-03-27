#pragma once

namespace simulation
{

// AI Will wander to adjacent tiles.
void
AIWander(Unit* unit)
{
  // Non interrupting behavior.
  if (unit->uaction != kUaNone) return;

  v2i p;
  if (!WorldToTilePos(unit->transform.position, &p)) return;

  v2i rpos = TileRandomNeighbor(p);
  v2f wpos = TilePosToWorld(rpos);
  v3f destination( wpos.x, wpos.y, 0.f );
  BB_SET(unit->bb, kUnitDestination, destination);
  unit->uaction = kUaMove;
}

void
AIDefend(Unit* unit)
{
  if (unit->uaction != kUaNone) return;

  Unit* target = GetNearestEnemyUnit(unit);
  if (!target) return;

  if (!CanPathTo(unit, target)) return;

  BB_SET(unit->bb, kUnitTarget, target->id);
  unit->uaction = kUaAttack;
}

// AI Will wander to adjacent tiles until it finds someone it can attack or
// a unit attacks it - in which case it will attack back.
void
AISimpleBehavior(Unit* unit)
{
  // Non interrupting behavior.
  if (unit->uaction != kUaNone) return;

  Unit* nearby_target = FindUnitInRangeToAttack(unit);
  if (nearby_target) {
    BB_SET(unit->bb, kUnitTarget, nearby_target->id);
    unit->uaction = kUaAttack;
    return;
  }

  AIWander(unit);
}

void
AIAttackInterrupt(Unit* unit)
{
  if (unit->uaction != kUaAttack) return;

  // If the AI discovers a nearer target than the one it is attacking it
  // will change targets.
  const uint32_t* current_target;
  if (!BB_GET(unit->bb, kUnitTarget, current_target)) return;

  Unit* nearest_target = GetNearestEnemyUnit(unit);
  if (!nearest_target) return;

  // Nearest target remains the one it is attacking.
  if (nearest_target->id == *current_target) return;
  // Otherwise switch to new nearest.
  BB_SET(unit->bb, kUnitTarget, nearest_target->id);
}

void
AIAttackWhenDiscovered(Unit* unit)
{
  AIAttackInterrupt(unit);

  // Non interrupting behavior.
  if (unit->uaction != kUaNone) return;

  v2i tpos;
  if (!WorldToTilePos(unit->transform.position, &tpos)) return;

  Tile* tile = TilePtr(tpos);
  if (!tile) return;
  if (tile->shroud) return;

  Unit* target = GetNearestEnemyUnit(unit);
  if (!target) return;

  BB_SET(unit->bb, kUnitTarget, target->id);
  unit->uaction = kUaAttack;
}

void
AIThink(Unit* unit)
{
  if (!unit) return;

  // Units should defend themselves when enemies are nearby.
  AIDefend(unit);

  const int* behavior;
  if (!BB_GET(unit->bb, kUnitBehavior, behavior)) return;
  switch (*behavior) {
    case kUnitBehaviorSimple: {
      AISimpleBehavior(unit);
    } break;
    case kUnitBehaviorAttackWhenDiscovered: {
      AIAttackWhenDiscovered(unit);
    } break;
    default: break;
  }

  // Clear AI knowledge that should be learned per tick.
  BB_REM(unit->bb, kUnitAttacker);
}

}  // namespace simulation
