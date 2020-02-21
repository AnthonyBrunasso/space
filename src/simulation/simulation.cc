#pragma once

#include <cstdio>

#include "platform/macro.h"
#include "platform/x64_intrin.h"

#include "entity.cc"
#include "ftl.cc"
#include "mhitu.cc"
#include "scenario.cc"
#include "search.cc"
#include "util.cc"

namespace simulation
{
constexpr float kDsqSelect = 25.f * 25.f;
constexpr float kDsqOperate = 50.f * 35.f;
constexpr float kDsqOperatePod = 75.f * 75.f;
constexpr float kAvoidanceScaling = 0.15f;
constexpr float kMovementScaling = 1.0f;
static uint64_t kSimulationHash = DJB2_CONST;

bool
Initialize()
{
  InitializeScenario();

  return true;
}

void
Reset()
{
  ResetScenario();
}

bool
VerifyIntegrity()
{
  for (int i = 0; i < kUsedRegistry; ++i) {
    uint64_t len = kRegistry[i].memb_size * kRegistry[i].memb_max;
    djb2_hash_more((const uint8_t*)kRegistry[i].ptr, len, &kSimulationHash);
  }

  return true;
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
ThinkAI()
{
  if (!kScenario.ai) return;

  for (uint64_t i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    Command c = {kUaNone};

    switch (unit->kind) {
      case Unit::kPlayerControlled:
        // Unit awaits human orders
        continue;
      case Unit::kPowerOperator:
        for (int k = 0; k < kUsedModule; ++k) {
          Module* mod = &kModule[k];
          if (mod->mod_power) c = (Command{kUaMove, v3fModule(mod), i});
        }
        break;
      case Unit::kMiner:
        for (int k = 0; k < kUsedModule; ++k) {
          Module* mod = &kModule[k];
          if (mod->mod_mine) c = (Command{kUaMove, v3fModule(mod), i});
        }
        break;
      case Unit::kEngineer:
        for (int k = 0; k < kUsedModule; ++k) {
          Module* mod = &kModule[k];
          if (mod->mod_engine) c = (Command{kUaMove, v3fModule(mod), i});
        }
        break;
      case Unit::kTurretOperator:
        for (int k = 0; k < kUsedModule; ++k) {
          Module* mod = &kModule[k];
          if (mod->mod_turret) c = (Command{kUaMove, v3fModule(mod), i});
        }
        break;
    };

    for (uint64_t j = 0; j < kUsedModule; ++j) {
      Module* mod = &kModule[j];
      if (v3fDsq(v3fModule(mod), unit->transform.position) < kDsqOperate) {
        c = (Command{kUaOperate, v3fModule(mod), i});
        break;
      }
    }

    PushCommand(c);
  }
}

void
ThinkShip()
{
  if (!kScenario.ship) return;

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
        for (int k = 0; k < kUsedModule; ++k) {
          Module* mod = &kModule[k];
          if (!mod->mod_power) continue;
          if (v3fDsq(unit->transform.position, v3fModule(mod)) <
              kDsqOperatePod) {
            // Visual
            Notify* n = UseNotify();
            n->age = 1;
            n->position = v3fModule(mod);

            if (operator_save_power(unit, kShip[i].power_delta)) {
              kShip[0].power_delta = 0.0f;
              LOGFMT(
                  "Unit %lu prevented the power surge from damaging the ship.",
                  unit - kUnit);
              break;
            }
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
    for (int j = 0; j < kUsedUnit; ++j) {
      Unit* unit = &kUnit[j];
      for (int k = 0; k < kUsedModule; ++k) {
        Module* mod = &kModule[k];
        if (mod->mod_power &&
            v3fDsq(unit->transform.position, v3fModule(mod)) < kDsqOperate)
          satisfied |= FLAG(kUnitAiPower);
      }
    }

    if (kShip[i].sys_power >= 1.0f) {
      for (int j = 0; j < kUsedUnit; ++j) {
        Unit* unit = &kUnit[j];
        for (int k = 0; k < kUsedModule; ++k) {
          Module* mod = &kModule[k];
          if (mod->mod_engine &&
              v3fDsq(unit->transform.position, v3fModule(mod)) < kDsqOperate)
            satisfied |= FLAG(kUnitAiThrust);
        }
      }

      for (int j = 0; j < kUsedUnit; ++j) {
        Unit* unit = &kUnit[j];
        for (int k = 0; k < kUsedModule; ++k) {
          Module* mod = &kModule[k];
          if (mod->mod_mine &&
              v3fDsq(unit->transform.position, v3fModule(mod)) < kDsqOperatePod)
            satisfied |= FLAG(kUnitAiMine);
        }
      }

      for (int j = 0; j < kUsedUnit; ++j) {
        Unit* unit = &kUnit[j];
        for (int k = 0; k < kUsedModule; ++k) {
          Module* mod = &kModule[k];
          if (mod->mod_turret &&
              v3fDsq(unit->transform.position, v3fModule(mod)) < kDsqOperate)
            satisfied |= FLAG(kUnitAiTurret);
        }
      }
    }

    kShip[i].crew_think_flags = satisfied;
  }
}

void
ThinkAsteroid()
{
  if (!kScenario.asteroid) return;

  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    asteroid->implode = (asteroid->mineral_source < .5f);
  }
}

void
ThinkMissle()
{
  if (!kScenario.missile) return;

  for (int i = 0; i < kUsedMissile; ++i) {
    Missile* missile = &kMissile[i];
    v2i tilepos = WorldToTilePos(missile->transform.position.xy());
    Tile* tile = TilePtr(tilepos);
    if (!tile) continue;

    // ship entering ftl, cannot strike
    if (kShip[0].ftl_frame) continue;

    if (tile->blocked) {
      v2i hack(0, 1);
      missile->explode_frame += 1;
      missile->y_velocity = 0;
      missile->tile_hit = tilepos + hack;
    }
  }
}

void
ThinkPod()
{
  if (!kScenario.pod) return;

  for (int i = 0; i < kUsedPod; ++i) {
    Pod* pod = &kPod[i];
    // Keep-state on AiReturn
    constexpr uint64_t keep = (FLAG(kPodAiReturn) | FLAG(kPodAiUnmanned));
    const uint64_t keep_state = pod->think_flags & keep;
    v2f goal;
    uint64_t think_flags = 0;

    // Goal is to return home, unless overidden
    for (int k = 0; k < kUsedModule; ++k) {
      Module* mod = &kModule[k];
      if (!mod->mod_mine) continue;
      goal = TilePosToWorld(v2i(mod->cx, mod->cy));
      break;
    }

    if (kShip[0].sys_mine < .5f) {
      think_flags |= FLAG(kPodAiLostPower);
    }

    // Stateful return home
    if (keep_state & FLAG(kPodAiUnmanned)) {
      think_flags = keep_state;
      // Waiting for spaceman in a spacesuit
      for (int j = 0; j < kUsedUnit; ++j) {
        if (!kUnit[j].spacesuit) continue;
        if (v3fDsq(kUnit[j].transform.position, pod->transform.position) <
            kDsqOperatePod) {
          LOGFMT("Unit %d is now in space.", j);
          think_flags = ANDN(FLAG(kPodAiUnmanned), think_flags);
          kUnit[j].inspace = 1;
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
        if (v3fDsq(goal, pod->transform.position) < 300.f) {
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
      uint64_t asteroid_index =
          v3fNear(pod->transform.position, GAME_ITER(Asteroid, transform));

      if (asteroid_index < kUsedAsteroid) {
        Asteroid* asteroid = &kAsteroid[asteroid_index];
        think_flags |= FLAG(kPodAiApproach);
        goal = asteroid->transform.position.xy();
        if (v3fDsq(asteroid->transform.position, pod->transform.position) <
            900.f) {
          think_flags |= FLAG(kPodAiGather);
          asteroid->deplete = 1;
        }
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
Think()
{
  ThinkAI();
  ThinkShip();
  ThinkAsteroid();
  ThinkMissle();
  ThinkPod();
}

void
DecideShip()
{
  if (!kScenario.ship) return;

  for (int i = 0; i < kUsedShip; ++i) {
    Ship* ship = &kShip[i];
    if (ship->danger > 20) {
      LOG("The ship's reactor is melting down.");
      ship->running = false;
    } else if (kUsedUnit < 1) {
      LOG("The crew is dead.");
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

    const bool jumped = (FtlSimulation(ship) == 0);
    // Jump side effects
    ship->mineral -= jumped * kFtlCost;
    ship->level += jumped;
  }
}

void
DecideAsteroid()
{
  if (!kScenario.asteroid) return;

  if (!kUsedAsteroid) {
    Asteroid* asteroid = UseAsteroid();
    asteroid->transform = Transform{.position = v3f(800.f, 750.f, 0.f)};
    asteroid->mineral_source = 200.f;
    asteroid->deplete = 0;
    asteroid->implode = 0;
  }

  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];

    if (asteroid->implode) {
      LOG("Asteroid imploded.");
      *asteroid = kZeroAsteroid;
      continue;
    }

    asteroid->mineral_source -= asteroid->deplete;
    asteroid->transform.scale =
        v3f(1.f, 1.f, 1.f) * (asteroid->mineral_source / 200.f);
    asteroid->transform.position.x -= 1.0f;
    if (asteroid->transform.position.x < 0.f) {
      asteroid->transform.position.x = 800.f;
    }
  }
}

void
DecideMissle()
{
  if (!kScenario.missile) return;

  const float missile_xrange = 50.f * kShip[0].level;
  static float next_missile = 0.f;
  while (kUsedMissile < kShip[0].level) {
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
          kShip[0].crew_think_flags & FLAG(kUnitAiTurret);

      if (laser_defense || !MissileHitSimulation(missile)) {
        *missile = kZeroMissile;
      }
    }

    missile->transform.position += v3f(0.0f, float(missile->y_velocity), 0.f);
  }
}

void
DecidePod()
{
  if (!kScenario.pod) return;

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

void
Decide()
{
  while (CountCommand()) {
    Command c = PopCommand();
    Unit* unit = &kUnit[c.unit];
    // Kind 0 accepts newest orders
    if (unit->kind * unit->uaction) continue;
    unit->uaction = c.type;
    unit->data = c.data;
  }

  DecideShip();
  DecideAsteroid();
  DecideMissle();
  DecidePod();
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
  // Advance ftl_frame, if active
  kShip[0].ftl_frame += (kShip[0].ftl_frame > 0);

  Think();
  Decide();

  // TODO (AN): Where does this reset really belong
  UpdateTilemap(kScenario.tilemap);

  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    Transform* transform = &unit->transform;
    v2i tilepos = WorldToTilePos(transform->position.xy());
    Tile* tile = TilePtr(tilepos);

    if (!tile) {
      *unit = kZeroUnit;
      continue;
    }

    // Reveal the shroud
    Tile keep_bits;
    memset(&keep_bits, 0xff, sizeof(Tile));
    keep_bits.shroud = 0;
    Tile set_bits;
    memset(&set_bits, 0x00, sizeof(Tile));
    set_bits.explored = 1;
    float tile_world_distance = kTileWidth * 2.0f;
    BfsMutate(unit->transform.position, keep_bits, set_bits,
              tile_world_distance * tile_world_distance);

    // Crew has been sucked away into the vacuum
    if (tile->nooxygen) {
      unit->uaction = kUaVacuum;
    }

    if (unit->uaction == kUaVacuum) {
      transform->position += TileVacuum(tilepos) * 3.0f;
    } else if (unit->uaction == kUaMove) {
      v2i end = WorldToTilePos(unit->data.destination);

      auto* path = PathTo(tilepos, end);
      if (!path) continue;

      bool near_goal = (path->size == 1);
      v3f move_dir = TileAvoidWalls(tilepos) * !near_goal;
      move_dir *= kAvoidanceScaling;
      if (v3fDsq(unit->transform.position, unit->data.destination) < 1.f) {
        unit->transform.position = unit->data.destination;
        continue;
      }

      move_dir +=
          math::Normalize(unit->data.destination - transform->position.xy()) *
          kMovementScaling;
      transform->position += move_dir;
    }
  }

  for (int i = 0; i < kUsedNotify; ++i) {
    Notify* n = &kNotify[i];
    if (POPCNT(n->age) == kNotifyAgeBits) {
      *n = kZeroNotify;
    } else {
      n->age += (n->age > 0);
    }
  }

  RegistryCompact();
}  // namespace simulation

}  // namespace simulation
