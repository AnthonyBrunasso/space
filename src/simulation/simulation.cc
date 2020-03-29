#pragma once

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "platform/macro.h"
#include "platform/x64_intrin.h"

#include "entity.cc"
#include "ftl.cc"
#include "mhitu.cc"
#include "phitu.cc"

#include "module.cc"

#include "scenario.cc"
#include "search.cc"
#include "selection.cc"
#include "util.cc"

#include "platform/macro.h"


#include "ai.cc"
namespace simulation
{
constexpr float kDsqGather = 25.f * 25.f;
constexpr float kDsqOperate = 50.f * 35.f;
constexpr float kDsqOperatePod = 75.f * 75.f;
constexpr float kAvoidanceScaling = 0.15f;
static uint64_t kSimulationHash = DJB2_CONST;
static bool kSimulationOver = false;

void
Reset(uint64_t seed)
{ 
  srand(seed);
  ScenarioReset(true);
}

bool
Initialize(uint64_t seed)
{
  Reset(seed);
  return true;
}

void
Hash()
{
  for (int i = 0; i < kUsedRegistry; ++i) {
    uint64_t len = kRegistry[i].memb_size * kRegistry[i].memb_max;
    djb2_hash_more((const uint8_t*)kRegistry[i].ptr, len, &kSimulationHash);
#ifdef DEBUG_SYNC
    printf("[ Type %d ] [ hash %016lx ]\n", i, kSimulationHash);
#endif
  }
}

bool
operator_save_power(Unit* unit, float power_delta)
{
  uint8_t int_check = power_delta / 5.0;
  bool success = (unit->acurrent[CREWA_INT] > int_check);
  LOGFMT("%u intelligence check on power utilization %04.02f [%d]", int_check,
         power_delta, success);
  // On success, update the known crew intelligence
  unit->aknown_min[CREWA_INT] =
      MAX(unit->aknown_min[CREWA_INT], success * int_check);
  return success;
}

void
ThinkShip(uint64_t ship_index)
{
  if (!kScenario.ship) return;

  Ship* ship = &kShip[ship_index];
  uint64_t think_flags = 0;

  // Ship mining is powererd
  if (ship->sys[kModMine] >= 1.0f) {
    if (ship->pod_capacity) {
      think_flags |= FLAG(kShipAiSpawnPod);
    }
  }

  if (ship->power_delta > 0.0f) {
    think_flags |= FLAG(kShipAiPowerSurge);
    ship->danger += 1;
    for (int j = 0; j < kUsedUnit; ++j) {
      Unit* unit = &kUnit[j];
      for (int k = 0; k < kUsedModule; ++k) {
        Module* mod = &kModule[k];
        if (mod->mkind != kModPower) continue;
        if (v3fDsq(unit->transform.position, v3fModule(mod)) < kDsqOperatePod) {
          if (operator_save_power(unit, ship->power_delta)) {
            unit->notify = 1;

            ship->power_delta = 0.0f;
            LOGFMT("Unit %u prevented the power surge from damaging ship %i.",
                   unit->id, ship_index);
            break;
          } else {
            ship->danger += 1;
          }
        }
      }
    }
  } else {
    ship->danger = 0;
  }
  ship->think_flags = think_flags;

  // Crew objectives
  uint64_t satisfied = 0;
  v2i module_position;
  for (int j = 0; j < kUsedUnit; ++j) {
    Unit* unit = &kUnit[j];
    for (int k = 0; k < kUsedModule; ++k) {
      Module* mod = &kModule[k];
      if (mod->mkind != kModPower && ship->sys[kModPower] < 1.0f) continue;

      if (v3fDsq(unit->transform.position, v3fModule(mod)) < kDsqOperate) {
        satisfied |= FLAG(mod->mkind);
      }
    }
  }

  ship->operate_flags = satisfied;
}

void
ThinkAsteroid()
{
  if (!kScenario.asteroid) return;
  //printf("%i\n", kUsedAsteroid);
  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    asteroid->implode = (asteroid->mineral_source < .5f);
  }
}

void
ThinkMissle(uint64_t ship_index)
{
  if (!kScenario.missile) return;

  for (int i = 0; i < kUsedMissile; ++i) {
    Missile* missile = &kMissile[i];
    v2i tilepos;
    if (!WorldToTilePos(missile->transform.position.xy(), &tilepos)) continue;
    Tile* tile = TilePtr(tilepos);
    if (!tile) continue;

    // ship entering ftl, cannot strike
    if (kShip[ship_index].ftl_frame) continue;

    if (tile->blocked) {
      v2i hack(0, 1);
      missile->explode_frame += 1;
      missile->y_velocity = 0;
      missile->tile_hit = tilepos + hack;
    }
  }
}

void
Think()
{
  ThinkAsteroid();

  for (uint64_t i = 0; i < kUsedShip; ++i) {
    TilemapSet(kShip[i].grid_index);
    // Shroud is reset each frame
    TilemapUpdate();

    ThinkShip(i);
    ThinkMissle(i);
  }
}

void
DecideShip(uint64_t ship_index)
{
  if (!kScenario.ship) return;

  // Advance engine animation
  kShip[ship_index].engine_animation += 1;
  // Advance ftl_frame, if active
  kShip[ship_index].ftl_frame += (kShip[ship_index].ftl_frame > 0);

  Ship* ship = &kShip[ship_index];
  for (int i = 0; i < kUsedModule; ++i) {
    Module* module = &kModule[i];
    if (module->ship_index != ship_index) continue;
    if (!ModuleBuilt(module)) continue;
    ModuleUpdate(module);
  }

  for (int i = 0; i < kModCount; ++i) {
    if (ship->operate_flags & FLAG(i)) {
      ship->sys[i] += 0.01f;
    } else {
      ship->sys[i] -= 0.01f;
    }
    ship->sys[i] = CLAMPF(ship->sys[i], 0.0f, 1.0f);
  }

  float used_power = 0.f;
  used_power += 20.f * (ship->sys[kModMine] >= 0.3f);
  used_power += 40.f * (ship->sys[kModEngine] >= .3f);
  used_power += 20.f * (ship->sys[kModTurret] >= 0.3f);
  ship->power_delta = fmaxf(used_power - ship->used_power, ship->power_delta);
  ship->used_power = used_power;

  const bool jumped = (FtlSimulation(ship) == 0);
  // Jump side effects
  kShip[ship_index].level += jumped;
}

void
DecideAsteroid()
{
  if (!kScenario.asteroid) return;

  if (kUsedAsteroid != kUsedPlayer) {
    bool asteroid_spawned[kMaxGrid] = { false };
    // Spawn an asteroid on each tilemap.
    for (int i = 0; i < kUsedAsteroid; ++i) {
      Asteroid* asteroid = &kAsteroid[i];
      uint64_t grid = TilemapWorldToGrid(asteroid->transform.position);
      asteroid_spawned[grid] = true;
    }

    for (int i = 0; i < kMaxGrid; ++i) {
      if (asteroid_spawned[i]) continue;
      Asteroid* asteroid = UseAsteroid();
      TilemapModify mod(i);
      asteroid->transform.position =
          TilePosToWorld(v2i(kMapHeight - 1, kMapWidth - 1));
      asteroid->mineral_source = 200.f;
      asteroid->implode = 0;
    }
  }

  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];

    if (asteroid->implode || asteroid->mineral_source == 0) {
      LOG("Asteroid imploded.");
      *asteroid = kZeroAsteroid;
      continue;
    }

    asteroid->mineral_source -= asteroid->deplete;
    asteroid->transform.scale =
        v3f(1.f, 1.f, 1.f) * (asteroid->mineral_source / 200.f);
    asteroid->transform.position.x -= 1.0f;
    if (asteroid->transform.position.x < 50.f) {
      asteroid->transform.position.x = 800.f;
    }
  }
}

v2f
InvasionDirection(v3f invasion_pos)
{
  v3f c = TilemapWorldCenter();
  float d = FLT_MAX;
  for (int i = 0; i < kUsedGrid; ++i) {
    TilemapModify mod(i);
    v3f nc = TilemapWorldCenter();
    float nd = math::LengthSquared(nc - invasion_pos);
    if (nd < d) {
      c = nc;
      d = nd;
    }
  }
  return math::Normalize(c.xy() - invasion_pos.xy());
}

void
DecideInvasion()
{
  if (!kScenario.invasion) return;

  if (!kUsedInvasion) {
    // Find min and max bounds of all grids.
    v2f min = TilePosToWorldMin({0, 0});
    v2f max = TilePosToWorldMin({kMapWidth, kMapHeight});
    for (int i = 0; i < kUsedGrid; ++i) {
      TilemapModify mod(i);
      v2f nmin = TilePosToWorldMin({0, 0});
      v2f nmax = TilePosToWorldMin({kMapWidth, kMapHeight});
      if (math::LengthSquared(nmin) < math::LengthSquared(min)) {
        min = nmin;
      }
      if (math::LengthSquared(nmax) > math::LengthSquared(max)) {
        max = nmax;
      }
    }
    math::Rectf r(min.x, min.y, max.x - min.x, max.y - min.y);
    // Spawn the invasion at a random point on the exterior of a rect
    // consuming all the grids pushed out by a vector from the midpoint
    // of the nearest grid to it.
    v2f rp = math::RandomPointOnRect(r);
    v2f dir = InvasionDirection(v3f(rp.x, rp.y, 0.f));
    Invasion* invasion = UseInvasion();
    invasion->transform.position =
        v3f(rp.x, rp.y, 0.f) - v3f(dir.x, dir.y, 0.f) * 350.f;
  }

  for (int i = 0; i < kUsedInvasion; ++i) {
    Invasion* v = &kInvasion[i];
    v2i tp;
    if (!v->docked && WorldToTilePos(v->transform.position, &tp)) {
      Tile* tile = TilePtr(tp);
      if (tile->blocked) {
        v->docked = true;
        v->docked_tile = v2i(tile->cx, tile->cy);
      }
    }
    if (!v->docked) {
      v->transform.position += InvasionDirection(v->transform.position);
    } else if (v->unit_count == 0) {
      // Spawn the units from the invasion force!
      BfsIterator iter = BfsStart(v->docked_tile);
      int count = rand() % kMaxThisInvasion + 1;
      while (v->unit_count != count) {
        BfsNext(&iter);
        if (iter.tile->exterior) continue;
        v->unit_id[v->unit_count++] =
            SpawnEnemy(v2i(iter.tile->cx, iter.tile->cy),
                       TilemapWorldToGrid(v->transform.position));
      }
      if (kMaxThisInvasion < kMaxInvasionCount) {
        ++kMaxThisInvasion;
      }
    }

    if (!v->docked) continue;

    // Invasion over when all units are dead.
    bool all_dead = true;
    for (int i = 0; i < v->unit_count; ++i) {
      uint32_t id = v->unit_id[i];
      if (FindUnit(id)) {
        all_dead = false;
        break;
      }
    }

    if (all_dead) *v = kZeroInvasion;
  }
}

void
DecideMissle(uint64_t ship_index)
{
  if (!kScenario.missile) return;

  const float missile_xrange = 50.f * kShip[ship_index].level;
  static float next_missile = 0.f;
  while (kUsedMissile < kShip[ship_index].level) {
    Missile* missile = UseMissile();
    missile->transform =
        Transform{.position = v3f(300.f + next_missile, -1000.f, 0.f)};
    missile->y_velocity = 5;
    next_missile = fmodf(next_missile + 50.f, missile_xrange);
  }

  for (int i = 0; i < kUsedMissile; ++i) {
    Missile* missile = &kMissile[i];

    if (missile->explode_frame) {
      const bool laser_defense =
          kShip[ship_index].operate_flags & FLAG(kModTurret);

      if (laser_defense || !MissileHitSimulation(missile)) {
        *missile = kZeroMissile;
      }
    }

    missile->transform.position += v3f(0.0f, float(missile->y_velocity), 0.f);
  }
}

bool
MoveTowards(Unit* unit, v2i tilepos, v3f dest, UnitAction set_on_arrival)
{
  if (v3fDsq(unit->transform.position, dest) < 1.f) {
    unit->transform.position = dest;
    unit->uaction = set_on_arrival;
    BB_REM(unit->bb, kUnitDestination);
    return true;
  }

  v3f incremental_dest = dest;
  v3f avoidance_vec = {};
  if (!unit->inspace) {
    v2i end;
    if (!WorldToTilePos(dest, &end)) return false;
    auto* path = PathTo(tilepos, end);
    if (!path) {
      unit->uaction = set_on_arrival;
      BB_REM(unit->bb, kUnitDestination);
      return true;
    }

    if (path->size > 1) {
      incremental_dest = TilePosToWorld(path->tile[1]);
      avoidance_vec = TileAvoidWalls(tilepos) * kAvoidanceScaling;
    }
  }

  v3f move_vec =
      math::Normalize(incremental_dest.xy() - unit->transform.position.xy()) *
      unit->speed;

  unit->transform.position += (move_vec + avoidance_vec);

  return false;
}

void
AttackTarget(Unit* unit, Unit* target)
{
  BB_SET(target->bb, kUnitAttacker, unit->id);

  if (!ShouldAttack(unit, target)) {
    return;
  }

  // Shoot at the target if weapon is off cooldown.
  if (kFrame - unit->attack_frame < unit->attack_cooldown) {
    return;
  }

  ProjectileCreate(target, unit, 7.5f, unit->weapon_kind);
  unit->attack_frame = kFrame;
}

void
ApplyCommand(Unit* unit, const Command& c)
{
  unsigned persistent_action = kUaNone;

  unit->uaction = c.type;
  int ctype = c.type;
  switch (ctype) {
    case kUaMove: {
      BB_SET(unit->bb, kUnitDestination, c.destination);
    } break;
    case kUaAttack: {
      Unit* target = GetUnit(c.destination);
      if (!target) return;
      BB_SET(unit->bb, kUnitTarget, target->id);
    } break;
    case kUaAttackMove: {
      BB_SET(unit->bb, kUnitAttackDestination, c.destination);
      persistent_action = c.type;
    } break;
    case kUaBuild: {
      BB_SET(unit->bb, kUnitDestination, c.destination);
    } break;
  }

  unit->persistent_uaction = c.type;
}

void
UpdateModule(uint64_t ship_index)
{
  for (int i = 0; i < kUsedModule; ++i) {
    Module* m = &kModule[i];
    if (m->ship_index != ship_index) continue;
    if (m->mkind == kModPower) {
      v3f world = TilePosToWorld(m->tile);
      // Reveal the shroud
      Tile keep_bits;
      memset(&keep_bits, 0xff, sizeof(Tile));
      keep_bits.shroud = 0;
      Tile set_bits;
      memset(&set_bits, 0x00, sizeof(Tile));
      set_bits.explored = 1;
      float tile_world_distance =
          kTileWidth * 8.0f * kShip[m->ship_index].sys[kModPower];
      BfsMutate(world, keep_bits, set_bits,
                tile_world_distance * tile_world_distance);
    }
  }
}

void
UpdateUnit(uint64_t ship_index)
{
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    if (unit->ship_index != ship_index) continue;
    Transform* transform = &unit->transform;
    v2i tilepos;
    if (!WorldToTilePos(transform->position.xy(), &tilepos)) continue;
    Tile* tile = TilePtr(tilepos);

    if (unit->health < 0.f) {
      unit->dead = 1;
      continue;
    }

    if (!tile) {
      *unit = kZeroUnit;
      continue;
    }

    // Reveal the shroud
    if (unit->alliance != kEnemy) {
      Tile keep_bits;
      memset(&keep_bits, 0xff, sizeof(Tile));
      keep_bits.shroud = 0;
      Tile set_bits;
      memset(&set_bits, 0x00, sizeof(Tile));
      set_bits.explored = 1;
      float tile_world_distance = kTileWidth * 2.0f;
      BfsMutate(unit->transform.position, keep_bits, set_bits,
                3.f * tile_world_distance * tile_world_distance);
    }

    // Crew has been sucked away into the vacuum
    if (tile->nooxygen) {
      unit->uaction = kUaVacuum;
    }

    AIThink(unit);

    for (int i = 0; i < kUsedModule; ++i) {
      Module* m = &kModule[i];
      if (ModuleBuilt(m)) continue;
      if (ModuleNear(m, unit->transform.position)) {
        m->frames_building++;
      }
    }

    // Implementation of uaction should properly reset persistent_uaction
    // when they are completed. Otherwise units will never have their uaction
    // reset to kUaNone and will continue to execute an action until a player
    // has explicitly issued them a new command.
    if (unit->uaction == kUaNone) {
      unit->uaction = unit->persistent_uaction;
    } else if (unit->uaction == kUaVacuum) {
      transform->position += TileVacuum(tilepos) * 3.0f;
    } else if (unit->uaction == kUaMove) {
      const v3f* dest = nullptr;
      if (!BB_GET(unit->bb, kUnitDestination, dest)) {
        continue;
      }

      if (MoveTowards(unit, tilepos, *dest, kUaNone)) {
        unit->persistent_uaction = kUaNone;
      }
    } else if (unit->uaction == kUaAttack) {
      const uint32_t* target = nullptr;
      if (!BB_GET(unit->bb, kUnitTarget, target)) {
        continue;
      }

      Unit* target_unit = FindUnit(*target);
      if (!target_unit || target_unit->dead) {
        BB_REM(unit->bb, kUnitTarget);
        unit->uaction = kUaNone;
        unit->persistent_uaction = kUaNone;
        continue;
      }

      if (!InRange(unit->id, *target)) {
        // Go to your target.
        BB_SET(unit->bb, kUnitDestination, target_unit->transform.position);
        MoveTowards(unit, tilepos, target_unit->transform.position, kUaAttack);
        continue;
      }

      AttackTarget(unit, target_unit);
    } else if (unit->uaction == kUaAttackMove) {
      const v3f* dest = nullptr;
      if (!BB_GET(unit->bb, kUnitAttackDestination, dest)) {
        continue;
      }

      Unit* target_unit = FindUnitInRangeToAttack(unit);
      if (!target_unit) {
        if (MoveTowards(unit, tilepos, *dest, kUaNone)) {
          unit->persistent_uaction = kUaNone;
        }
        continue;
      }

      AttackTarget(unit, target_unit);
    } else if (unit->uaction == kUaBuild) {
      const v3f* dest = nullptr;
      if (!BB_GET(unit->bb, kUnitDestination, dest)) {
        continue;
      }

      if (MoveTowards(unit, tilepos, *dest, kUaNone)) {
        unit->persistent_uaction = kUaBuild;
      }
    }
  }

  // Unit death logic happens here
  for (int i = 0; i < kUsedUnit; ++i) {
    if (kUnit[i].dead) {
      uint32_t death_id = kUnit[i].id;
      LOGFMT("Unit died [id %d]", death_id);
      kUnit[i] = kZeroUnit;
    }
  }
}

void
UpdateConsumable(uint64_t ship_index)
{
  for (int i = 0; i < kUsedConsumable; ++i) {
    Consumable* c = &kConsumable[i];
    v3f cw = TilePosToWorld(v2i(c->cx, c->cy));
    float dsq;
    uint64_t near_unit = v3fNearTransform(cw, GAME_ITER(Unit, transform), &dsq);
    if (dsq < kDsqGather) {
      if (c->cryo_chamber) {
        const v3f scale = v3f(0.25f, 0.25f, 0.f);
        uint8_t attrib[CREWA_MAX] = {11, 10, 11, 10};
        Unit* new_unit = UseIdUnit();
        new_unit->ship_index = ship_index;
        new_unit->transform.position = cw;
        new_unit->transform.scale = scale;
        memcpy(new_unit->acurrent, attrib, sizeof(attrib));
        new_unit->kind = kOperator;
        // Everybody is unique!
        new_unit->mskill = rand() % kModCount;
        new_unit->player_id = AssignPlayerId();

        *c = kZeroConsumable;
      } else {
        // TODO: Fix this.
        //kResource[0].mineral += c->minerals;
        *c = kZeroConsumable;
      }
    }
  }
}

void
Decide()
{
  while (CountCommand()) {
    Command c = PopCommand();
    if (c.unit_id == kInvalidUnit) {
      if (c.type == kUaBuild) {
        for (int i = 0; i < kUsedUnit; ++i) {
          Unit* u = &kUnit[i];
          if (0 == (u->control & c.control)) continue;
          ApplyCommand(u, c);
        }
      } else {
        TilemapSet(TilemapWorldToGrid(c.destination));
        v2i start;
        if (!WorldToTilePos(c.destination, &start)) continue;
        BfsIterator iter = BfsStart(start);
        for (int i = 0; i < kUsedUnit; ++i) {
          // The issuer of a command must have a set bit
          if (0 == (kUnit[i].control & c.control)) continue;
          c.destination = TileToWorld(*iter.tile);
          ApplyCommand(&kUnit[i], c);
          BfsNext(&iter);
        }
      }
    } else {
      Unit* unit = FindUnit(c.unit_id);
      if (!unit) continue;
      // Unit specific commands must exactly match the original control bits
      if (unit->control != c.control) continue;
      // Unit is busy.
      if (unit->uaction != kUaNone) continue;
      ApplyCommand(unit, c);
    }
  }

  DecideAsteroid();
  DecideInvasion();

  for (uint64_t i = 0; i < kUsedShip; ++i) {
    TilemapSet(kShip[i].grid_index);
    DecideShip(i);
    DecideMissle(i);
    UpdateModule(i);
    UpdateUnit(i);
    UpdateConsumable(i);
  }
}

void
Update()
{
  ++kFrame;

  kSimulationOver = ScenarioOver();

  // Camera can move even when game is over
  for (int i = 0; i < kUsedPlayer; ++i) {
    camera::Update(&kPlayer[i].camera);
    kPlayer[i].camera.motion.z = 0.f;
  }

  if (kFrame == 1500) {
    kScenario.invasion = true;
    LOG("Invasions beginning!");
  }

  if (kSimulationOver) return;

  for (int i = 0; i < kUsedShip; ++i) {
    TilemapSet(kShip[i].grid_index);
  }

  Think();
  Decide();
  TilemapSet(-1);

  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* u = &kUnit[i];
    u->notify += (u->notify > 0);
  }

  ProjectileSimulation();

  RegistryCompact();
}  // namespace simulation

}  // namespace simulation
