#pragma once

#include <cstdio>

#include "platform/macro.h"
#include "platform/x64_intrin.h"

#include "entity.cc"
#include "ftl.cc"
#include "interaction.cc"
#include "mhitu.cc"
#include "search.cc"

namespace simulation
{
constexpr float kDsqOperate = 50.f * 35.f;
constexpr float kDsqOperatePod = 75.f * 75.f;

bool
Initialize()
{
  v3f pos[] = {v3f(300.f, 300.f, 0.f), v3f(100.f, 130.f, 0),
               v3f(300.f, 400.f, 0), v3f(650.f, 460.f, 0),
               v3f(100.f, 577.f, 0.f)};
  const v3f scale = v3f(0.25f, 0.25f, 0.f);
  uint8_t attrib[CREWA_MAX] = {11, 10, 11, 10};
  for (int i = 0; i < ARRAY_LENGTH(pos); ++i) {
    Unit* unit = UseUnit();
    unit->transform.position = pos[i];
    unit->transform.scale = scale;
    memcpy(unit->acurrent, attrib, sizeof(attrib));
    // Everybody is unique!
    unit->kind = i + 1;
  }
  kUnit[4].state_flags = FLAG(kUnitStateSpaceSuit);

  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    printf("UNIT %i: %.2f,%.2f\n", i, unit->transform.position.x,
           unit->transform.position.y);
  }

  UseShip();
  kShip[0].running = true;
  kShip[0].level = 1;

  InitializeTilemap();

  return true;
}

bool
VerifyIntegrity()
{
  // TODO (AN): Checksum of kEntity compared to end of last frame
  return true;
}

bool
operator_save_power(Unit* unit, float power_delta)
{
  uint8_t int_check = power_delta / 5.0;
#ifdef DEBUG_AI
  printf("%u int check to save_power %04.02f\n", int_check, power_delta);
#endif
  bool success = (unit->acurrent[CREWA_INT] > int_check);
  // On success, update the known crew intelligence
  unit->aknown_min[CREWA_INT] =
      MAX(unit->aknown_min[CREWA_INT], success * int_check);
  return success;
}

void
Think()
{
  for (int i = 0; i < kUsedUnit; ++i) {
    switch (kUnit[i].kind) {
      default:
      case 0:
        // Just takes orders
        kUnit[i].think_flags = 0;
        break;
      case 1:
        kUnit[i].think_flags = FLAG(kUnitAiPower);
        break;
      case 2:
        kUnit[i].think_flags = FLAG(kUnitAiMine);
        break;
      case 3:
        kUnit[i].think_flags = FLAG(kUnitAiThrust);
        break;
      case 4:
        kUnit[i].think_flags = FLAG(kUnitAiTurret);
        break;
    };
  }

  for (int i = 0; i < kUsedShip; ++i) {
    // Ship already has a pod, no-op
    uint64_t think_flags = 0;

    if (!kUsedPod) {
      // Ship mining is powererd
      if (kShip[i].sys_mine >= 1.0f) {
        think_flags |= FLAG(kShipAiSpawnPod);
      }
    }

    if (kShip[i].power_delta > 0.0f) {
      think_flags |= FLAG(kShipAiPowerSurge);
      kShip[i].danger += 1;
      for (int j = 0; j < kUsedUnit; ++j) {
        Unit* unit = &kUnit[j];
        v2i grid = TypeOnGrid(kTilePower);
        v2f power_module = TilePosToWorld(grid);
        if (dsq(unit->transform.position, power_module) < kDsqOperate) {
          if (operator_save_power(unit, kShip[i].power_delta)) {
            unit->think_flags |= FLAG(kUnitAiSavePower);
            break;
          }
        }
      }
    } else {
      kShip[i].danger = 0;
    }
    kShip[i].think_flags = think_flags;

    // Crew objectives
    uint64_t satisfied = 0;
    v2i module_position;
    module_position = TypeOnGrid(kTilePower);
    for (int j = 0; j < kUsedUnit; ++j) {
      if (dsq(kUnit[j].transform.position, TilePosToWorld(module_position)) <
          kDsqOperate)
        satisfied |= FLAG(kUnitAiPower);
    }

    if (kShip[i].sys_power >= 1.0f) {
      module_position = TypeOnGrid(kTileEngine);
      for (int j = 0; j < kUsedUnit; ++j) {
        if (dsq(kUnit[j].transform.position, TilePosToWorld(module_position)) <
            kDsqOperate)
          satisfied |= FLAG(kUnitAiThrust);
      }

      module_position = TypeOnGrid(kTileMine);
      for (int j = 0; j < kUsedUnit; ++j) {
        if (dsq(kUnit[j].transform.position, TilePosToWorld(module_position)) <
            kDsqOperate)
          satisfied |= FLAG(kUnitAiMine);
      }

      module_position = TypeOnGrid(kTileTurret);
      for (int j = 0; j < kUsedUnit; ++j) {
        if (dsq(kUnit[j].transform.position, TilePosToWorld(module_position)) <
            kDsqOperate)
          satisfied |= FLAG(kUnitAiTurret);
      }
    }

    kShip[i].crew_think_flags = satisfied;
  }

  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    if (asteroid->mineral_source < .5f) {
      asteroid->flags = FLAG(kAsteroidAiImplode);
    } else {
      asteroid->flags = 0;
    }
  }

  for (int i = 0; i < kUsedMissile; ++i) {
    Missile* missile = &kMissile[i];
    v2i tile = WorldToTilePos(missile->transform.position.xy());
    if (!TileOk(tile)) continue;

    // ship entering ftl, cannot strike
    if (kShip[0].frame - kShip[0].ftl.frame > 1) continue;

    if (kTilemap.map[tile.y][tile.x].type == kTileBlock) {
      missile->flags = FLAG(kMissileAiExplode);
      missile->tile_hit = {tile.x, (tile.y + 1)};
    }
  }

  for (int i = 0; i < kUsedPod; ++i) {
    Pod* pod = &kPod[i];
    // Keep-state on AiReturn
    constexpr uint64_t keep = (FLAG(kPodAiReturn) | FLAG(kPodAiUnmanned));
    const uint64_t keep_state = pod->think_flags & keep;
    v2f goal;
    uint64_t think_flags = 0;

    // Goal is to return home, unless overidden
    v2i grid = TypeOnGrid(kTileMine);
    goal = TilePosToWorld(grid);

    if (kShip[0].sys_mine < .5f) {
      think_flags |= FLAG(kPodAiLostPower);
    }

    // Stateful return home
    if (keep_state & FLAG(kPodAiUnmanned)) {
      think_flags = keep_state;
      // Waiting for spaceman in a spacesuit
      for (int j = 0; j < kUsedUnit; ++j) {
        if (0 == (kUnit[j].state_flags & FLAG(kUnitStateSpaceSuit))) continue;
        if (dsq(kUnit[j].transform.position, pod->transform.position) <
            kDsqOperatePod) {
          printf("Crew in space %d\n", j);
          think_flags = ANDN(FLAG(kPodAiUnmanned), think_flags);
          kUnit[j].state_flags |= FLAG(kUnitStateInSpace);
          break;
        }
      }
    } else if (keep_state & FLAG(kPodAiReturn)) {
      // Pod has finished unloading
      if (pod->mineral == 0) {
        // derp
        think_flags = ANDN(FLAG(kPodAiUnload), think_flags);
      } else {
        think_flags = keep_state;
        // In range of the ship
        if (dsq(goal, pod->transform.position) < 300.f) {
          // Unload the payload this tick!
          think_flags |= FLAG(kPodAiUnload);
        }
      }
    }
    // Begin the journey home
    else if (pod->mineral >= kPodMaxMineral) {
      v2f home;
      goal = home;
      think_flags |= FLAG(kPodAiReturn);
    } else {
      // Evaluate mining potential
      for (int i = 0; i < kUsedAsteroid; ++i) {
        Asteroid* asteroid = &kAsteroid[i];
        if (asteroid->mineral_source == 0) continue;

        // TODO: nearest dsq
        think_flags |= FLAG(kPodAiApproach);
        goal = asteroid->transform.position.xy();
        if (transform_dsq(&asteroid->transform, &pod->transform) < 900.f) {
          think_flags |= FLAG(kPodAiGather);
          asteroid->flags |= FLAG(kAsteroidAiDeplete);
        }
        break;
      }
    }

    pod->think_flags = think_flags;
    pod->goal = goal;
#if DEBUG_AI
    printf("pod think 0x%lx keep_state 0x%lx minerals %lu \n", think_flags,
           keep_state, pod->mineral);
#endif
  }
}

void
Decide()
{
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    // Busy unit
    if (unit->command.type != Command::kNone) {
      continue;
    }

    // Obedient Unit
    if (unit->think_flags == 0) {
      if (CountCommand()) {
        unit->command = PopCommand();
      }
      continue;
    }

    // hero saves the day!
    if (unit->think_flags & FLAG(kUnitAiSavePower)) {
      kShip[0].power_delta = 0.0f;
#ifdef DEBUG_AI
      printf("Unit %lu prevented the power surge from damaging the ship\n",
             unit - kUnit);
#endif
      continue;
    }

    // Self-motivated Unit
    uint64_t possible = ANDN(kShip[0].crew_think_flags, unit->think_flags);
    if (!possible) continue;

    uint64_t action = TZCNT(possible);
#if DEBUG_AI
    printf(
        "[ %d unit ] [ 0x%lx think ] [ 0x%lx possible ] ship crew_think_flags "
        "0x%lx \n",
        i, unit->think_flags, possible, kShip[0].crew_think_flags);
    printf("%lu action\n", action);
#endif
    TileType type;
    switch (action) {
      case kUnitAiMine:
        type = kTileMine;
        break;
      case kUnitAiPower:
        type = kTilePower;
        break;
      case kUnitAiThrust:
        type = kTileEngine;
        break;
      case kUnitAiTurret:
        type = kTileTurret;
        break;
    };

    v2i grid = AdjacentOnGrid(type);
    v2f pos = TilePosToWorld(grid);
    unit->command = Command{.type = Command::kMove, .destination = pos};
  }

  for (int i = 0; i < kUsedShip; ++i) {
    Ship* ship = &kShip[i];

    if (ship->danger > 20) {
      puts("ship danger triggered game over");
      ship->running = false;
    }
    if (ship->think_flags & FLAG(kShipAiSpawnPod)) {
      Pod* pod = UsePod();
      pod->transform = Transform{.position = v3f(520.f, 600.f, 0.f)};
      pod->think_flags = FLAG(kPodAiUnmanned);
    }

    if (ship->crew_think_flags & FLAG(kUnitAiPower))
      ship->sys_power += 0.01f;
    else
      ship->sys_power += -0.01f;
    if (ship->crew_think_flags & FLAG(kUnitAiMine))
      ship->sys_mine += 0.01f;
    else
      ship->sys_mine += -0.01f;
    if (ship->crew_think_flags & FLAG(kUnitAiThrust))
      ship->sys_engine += 0.01f;
    else
      ship->sys_engine += -0.01f;
    if (ship->crew_think_flags & FLAG(kUnitAiTurret))
      ship->sys_turret += 0.01f;
    else
      ship->sys_turret += -0.01f;

    ship->sys_power = CLAMPF(ship->sys_power, 0.0f, 1.0f);
    ship->sys_mine = CLAMPF(ship->sys_mine, 0.0f, 1.0f);
    ship->sys_engine = CLAMPF(ship->sys_engine, 0.0f, 1.0f);
    ship->sys_turret = CLAMPF(ship->sys_turret, 0.0f, 1.0f);

    float used_power = 0.f;
    used_power += 20.f * (ship->sys_mine >= 0.3f);
    used_power += 40.f * (ship->sys_engine >= .3f);
    used_power += 20.f * (ship->sys_turret >= 0.3f);
    ship->power_delta = fmaxf(used_power - ship->used_power, ship->power_delta);
    ship->used_power = used_power;

    bool jumped = FtlUpdate(ship, &ship->ftl);
    // Jump side effects
    ship->mineral -= jumped * kFtlCost;
    ship->level += jumped;
  }

  if (!kUsedAsteroid) {
    Asteroid* asteroid = UseAsteroid();
    asteroid->transform = Transform{.position = v3f(800.f, 750.f, 0.f)};
    asteroid->mineral_source = 200.f;
    asteroid->flags = 0;
  }

  const float missile_xrange = 50.f * kShip[0].level;
  static float next_missile = 0.f;
  while (kUsedMissile < kShip[0].level) {
    Missile* missile = UseMissile();
    missile->transform =
        Transform{.position = v3f(300.f + next_missile, -1000.f, 0.f)};
    missile->flags = FLAG(kMissileAiFlight);
    next_missile = fmodf(next_missile + 50.f, missile_xrange);
  }

  for (int i = 0; i < kUsedMissile; ++i) {
    Missile* missile = &kMissile[i];
    uint64_t action = TZCNT(missile->flags);
    uint64_t replaced;

    if (action == kMissileAiExplode) {
      MissileHitShip(&kShip[0], missile);
      *missile = kZeroMissile;
      continue;
    }

    if (action != kMissileAiFlight) continue;

    missile->transform.position += v3f(0.0f, 5.f, 0.f);
  }

  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    uint64_t action = TZCNT(asteroid->flags);

    if (action == kAsteroidAiImplode) {
      printf("zero asteroid %d\n", i);
      *asteroid = kZeroAsteroid;
      continue;
    }

    asteroid->mineral_source -= (action == kAsteroidAiDeplete);
    asteroid->transform.scale =
        v3f(1.f, 1.f, 1.f) * (asteroid->mineral_source / 200.f);
    asteroid->transform.position.x -= 1.0f;
    if (asteroid->transform.position.x < 0.f) {
      asteroid->transform.position.x = 800.f;
    }
  }

  for (int i = 0; i < kUsedPod; ++i) {
    Pod* pod = &kPod[i];

    uint64_t action = TZCNT(pod->think_flags);
    uint64_t mineral = 0;
    v3f dir = v3f();
    switch (action) {
      case kPodAiLostPower:
        dir = pod->last_heading;
        break;
      case kPodAiApproach:
        dir = Normalize(pod->goal - pod->transform.position.xy());
        break;
      case kPodAiGather:
        mineral = MIN(1, kPodMaxMineral - pod->mineral);
        pod->mineral += mineral;
        break;
      case kPodAiReturn:
        dir = Normalize(pod->goal - pod->transform.position.xy());
        break;
      case kPodAiUnload:
        kShip[0].mineral += MIN(5, pod->mineral);
        pod->mineral -= MIN(5, pod->mineral);
        break;
      case kPodAiUnmanned:
        break;
    };

#ifdef DEBUG_AI
    printf("pod ai [ action %lu ] [ think %lu ] [dir %f %f]\n", action,
           pod->think_flags, dir.x, dir.y);
#endif
    pod->transform.position += dir * 2.0;
    pod->last_heading = dir.xy();
  }
}

bool
SimulationOver()
{
  return !(kShip[0].running);
}

void
Update()
{
  if (!kShip[0].running) return;
  kShip[0].frame += 1;

  Think();
  Decide();

  for (int i = 0; i < kUsedUnit;) {
    Unit* unit = &kUnit[i];
    Transform* transform = &unit->transform;
    v2i tilepos = WorldToTilePos(transform->position.xy());

    if (!TileOk(tilepos)) {
      *unit = kZeroUnit;
      continue;
    }

    if (unit->vacuum == v3f()) {
      // Crew has been sucked away into the vacuum
      if (kTilemap.map[tilepos.y][tilepos.x].type == kTileVacuum) {
        unit->vacuum = TileVacuum(tilepos);
        unit->command.type = Command::kVacuum;
      }
    }

    switch (unit->command.type) {
      case Command::kNone: {
      } break;
      case Command::kMine: {
      } break;
      case Command::kMove: {
        v2i end = WorldToTilePos(unit->command.destination);

        auto* path = PathTo(tilepos, end);
        if (!path || path->size <= 1) {
          unit->command = {};
          break;
        }

        v3f dest = TilePosToWorld(path->tile[1]);
        auto dir = math::Normalize(dest - transform->position.xy());
        transform->position += (dir * 1.f) + (TileAvoidWalls(tilepos) * .15f);
      } break;
      case Command::kVacuum: {
        transform->position += (unit->vacuum * 1.5f);
      } break;
      default:
        break;
    }

    ++i;
  }

  RegistryCompact();
}

}  // namespace simulation
