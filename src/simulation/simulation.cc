#pragma once

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "util.cc"

#include "ai.cc"
#include "entity.cc"
#include "ftl.cc"
#include "module.cc"
#include "phitu.cc"
#include "scenario.cc"
#include "search.cc"
#include "selection.cc"
#include "ship.cc"

namespace simulation
{
constexpr float kDsqGather = 25.f * 25.f;
constexpr float kDsqOperate = 50.f * 35.f;
constexpr float kDsqOperatePod = 75.f * 75.f;
constexpr float kAvoidanceScaling = 0.15f;
constexpr uint64_t kTileVisibleDistance = 3;
static uint64_t kSimulationHash = DJB2_CONST;
static bool kSimulationOver = false;

void
Reset(uint64_t seed)
{
  srand(seed);
  ScenarioReset();
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

void
TilemapUpdate()
{
  if (GAME_SHROUD) {
    TilemapResetVisible();
  }

  // Logical isolation for mapping v3f -> tile
  // Copying the tile introduces a frame delay when processing tile properties
  FOR_EACH_ENTITY(Unit, unit, { WorldToTile(unit->position, &unit->tile); });
}

void
DecideShip(uint64_t ship_index)
{
  // Advance engine animation
  kShip[ship_index].engine_animation += 1;
  // Advance ftl_frame, if active
  kShip[ship_index].ftl_frame += (kShip[ship_index].ftl_frame > 0);

  Ship* ship = &kShip[ship_index];
  FOR_EACH_ENTITY(Module, module, {
    if (module->ship_index != ship_index) continue;
    if (!ModuleBuilt(module)) continue;
    ModuleUpdate(module);
  });

  const bool jumped = (FtlSimulation(ship) == 0);
  // Jump side effects
  kShip[ship_index].level += jumped;
}

void
DecideAsteroid()
{
  if (kUsedAsteroid != kUsedPlayer) {
    bool asteroid_spawned[kMaxGrid] = {false};
    // Spawn an asteroid on each tilemap.
    for (int i = 0; i < kUsedAsteroid; ++i) {
      Asteroid* asteroid = &kAsteroid[i];
      uint64_t grid = TilemapWorldToGrid(asteroid->transform.position);
      asteroid_spawned[grid] = true;
    }

    for (int i = 0; i < kPlayerCount; ++i) {
      if (asteroid_spawned[i]) continue;
      Asteroid* asteroid = UseAsteroid();
      Rectf ship_bounds = ShipBounds(i);
      asteroid->transform.position = v2f(ship_bounds.x + ship_bounds.width,
                                         ship_bounds.y + ship_bounds.height);
      asteroid->mineral_source = 200.f;
    }
  }

  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];

    if (asteroid->mineral_source < .5f) {
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

void
DecideInvasion()
{
  /*if (kFrame < 1500) {
    return;
  } else if (kFrame == 1500) {
    LOG("Invasions have begun");
  }*/

  if (kUsedInvasion != kMaxPlayer) {
    Rectf r = FleetBounds();
    // Spawn the invasion at a random point on the exterior of a rect
    // consuming all the grids pushed out by a vector from the midpoint
    // of the nearest grid to it.
    v2f rp = math::RandomPointOnRect(r);
    v2f center(r.x + .5 * r.width, r.y + .5 * r.height);
    v2f invasion_dir = math::Normalize(center - rp);

    Invasion* invasion = UseInvasion();
    invasion->transform.position =
        center - invasion_dir * fmax(r.width, r.height);
    invasion->invasion_dir = invasion_dir;
  }

  for (int i = 0; i < kUsedInvasion; ++i) {
    Invasion* v = &kInvasion[i];
    TilemapModify mod(TilemapWorldToGrid(v->transform.position));
    Tile tile;
    if (!v->docked && WorldToTile(v->transform.position, &tile)) {
      if (tile.blocked) {
        v->docked = true;
        v->docked_tile = tile;
      }
    }
    if (!v->docked) {
      v->transform.position += v->invasion_dir;
    } else if (v->unit_count == 0) {
      // Spawn the units from the invasion force!
      BfsIterator iter = BfsStart(v->docked_tile);
      int count = rand() % kMaxThisInvasion + 1;
      while (BfsNext(&iter)) {
        if (iter.tile->exterior || iter.tile->blocked) continue;
        v->unit_id[v->unit_count++] =
            SpawnEnemy(v2i(iter.tile->cx, iter.tile->cy),
                       TilemapWorldToGrid(v->transform.position));

        if (v->unit_count == count) break;
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
      if (FindEntity(id)) {
        all_dead = false;
        break;
      }
    }

    if (all_dead) *v = kZeroInvasion;
  }
}

bool
MoveTowards(Unit* unit, Tile dest, UnitAction set_on_arrival)
{
  if (unit->tile.cxy == dest.cxy) return true;

  Tile incremental_dest = dest;
  v3f avoidance_vec = {};
  if (!unit->inspace) {
    auto* path = PathTo(unit->tile, dest);
    if (!path) {
      unit->uaction = set_on_arrival;
      BB_REM(unit->bb, kUnitDestination);
      return true;
    }

    if (path->size > 1) {
      incremental_dest.cx = path->tile[1].x;
      incremental_dest.cy = path->tile[1].y;
      avoidance_vec = TileAvoidWalls(unit->tile) * kAvoidanceScaling;
    }
  }

  v3f delta(incremental_dest.cx - unit->tile.cx,
            incremental_dest.cy - unit->tile.cy, 0.f);
  v3f move_vec = delta * unit->speed;

  unit->position += (move_vec + avoidance_vec);

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
      Tile t;
      if (WorldToTile(c.destination, &t)) {
        BB_SET(unit->bb, kUnitDestination, t);
      }
    } break;
    case kUaAttack: {
      Unit* target = GetUnit(c.destination);
      if (!target) return;
      BB_SET(unit->bb, kUnitTarget, target->id);
    } break;
    case kUaAttackMove: {
      Tile t;
      if (WorldToTile(c.destination, &t)) {
        BB_SET(unit->bb, kUnitAttackDestination, t);
        persistent_action = c.type;
      }
    } break;
    case kUaBuild: {
      Tile t;
      if (WorldToTile(c.destination, &t)) {
        BB_SET(unit->bb, kUnitDestination, t);
      }
    } break;
  }

  unit->persistent_uaction = c.type;
}

void
UpdateModule(uint64_t ship_index)
{
  TilemapModify tm(ship_index);
  FOR_EACH_ENTITY(Module, m, {
    if (m->ship_index != ship_index) continue;
    if (m->mkind != kModPower) continue;

    Tile tile;
    tile.cx = m->tile.cx;
    tile.cy = m->tile.cy;
    tile.visible = 1;
    BfsTileEnable(tile, kMapWidth);
    break;
  });
}

void
UpdateUnit(uint64_t ship_index)
{
  FOR_EACH_ENTITY(Unit, unit, {
    if (unit->ship_index != ship_index) continue;

    if (unit->health < 0.f) {
      unit->dead = 1;
      continue;
    }

    // Reveal the shroud
    if (unit->alliance != kEnemy) {
      Tile set_tile;
      set_tile.cx = unit->tile.cx;
      set_tile.cy = unit->tile.cy;
      set_tile.visible = 1;
      set_tile.explored = 1;
      BfsTileEnable(set_tile, kTileVisibleDistance);
    }

    AIThink(unit);

    FOR_EACH_ENTITY(Module, m, {
      if (ModuleBuilt(m)) continue;
      if (ModuleNear(m, unit->position)) {
        m->frames_building++;
      }
    });

    // Implementation of uaction should properly reset persistent_uaction
    // when they are completed. Otherwise units will never have their uaction
    // reset to kUaNone and will continue to execute an action until a player
    // has explicitly issued them a new command.
    if (unit->uaction == kUaNone) {
      unit->uaction = unit->persistent_uaction;
    } else if (unit->uaction == kUaMove) {
      const Tile* dest = nullptr;
      if (!BB_GET(unit->bb, kUnitDestination, dest)) {
        continue;
      }

      if (MoveTowards(unit, *dest, kUaNone)) {
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
        BB_SET(unit->bb, kUnitDestination, target_unit->tile);
        MoveTowards(unit, target_unit->tile, kUaAttack);
        continue;
      }

      AttackTarget(unit, target_unit);
    } else if (unit->uaction == kUaAttackMove) {
      const Tile* dest = nullptr;
      if (!BB_GET(unit->bb, kUnitAttackDestination, dest)) {
        continue;
      }

      Unit* target_unit = FindUnitInRangeToAttack(unit);
      if (!target_unit) {
        if (MoveTowards(unit, *dest, kUaNone)) {
          unit->persistent_uaction = kUaNone;
        }
        continue;
      }

      AttackTarget(unit, target_unit);
    } else if (unit->uaction == kUaBuild) {
      const Tile* dest = nullptr;
      if (!BB_GET(unit->bb, kUnitDestination, dest)) {
        continue;
      }

      if (MoveTowards(unit, *dest, kUaNone)) {
        unit->persistent_uaction = kUaBuild;
      }
    }
  });

  // Unit death logic happens here
  FOR_EACH_ENTITY(Unit, unit, {
    if (unit->dead) {
      uint32_t death_id = unit->id;
      LOGFMT("Unit died [id %d]", death_id);
      ZeroEntity(unit);
    }
  });
}

void
Decide()
{
  while (CountCommand()) {
    Command c = PopCommand();
    if (c.unit_id == kInvalidId) {
      if (c.type == kUaBuild) {
        FOR_EACH_ENTITY(Unit, unit, {
          if (0 == (unit->control & c.control)) continue;
          ApplyCommand(unit, c);
        });
      } else {
        TilemapModify tm(TilemapWorldToGrid(c.destination));
        Tile start;
        if (!WorldToTile(c.destination, &start)) continue;
        BfsIterator iter = BfsStart(start);
        FOR_EACH_ENTITY(Unit, unit, {
          // The issuer of a command must have a set bit
          if (0 == (unit->control & c.control)) continue;
          c.destination = TileToWorld(*iter.tile);
          ApplyCommand(unit, c);
          if (!BfsNextTile(&iter)) break;
        });
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
    TilemapModify tm(i);
    DecideShip(i);
    UpdateModule(i);
    UpdateUnit(i);
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

    if (kPlayer[i].mineral_cheat) {
      kPlayer[i].mineral = 99999;
    }
  }

  if (kSimulationOver) return;

  TilemapUpdate();
  Decide();
  TilemapClear();

  FOR_EACH_ENTITY(Unit, unit, {
    if (!unit) continue;
    unit->notify += (unit->notify > 0);
  });

  ProjectileSimulation();

  RegistryCompact();
}  // namespace simulation

}  // namespace simulation
