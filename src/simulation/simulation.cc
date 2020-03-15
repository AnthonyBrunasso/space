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
  ResetScenario(true);
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

  if (!kUsedPod) {
    // Ship mining is powererd
    if (ship->sys[kModMine] >= 1.0f) {
      if (ship->pod_capacity) {
        think_flags |= FLAG(kShipAiSpawnPod);
        ship->pod_capacity -= 1;
      }
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
            // Visual
            Notify* n = UseNotify();
            n->age = 1;
            n->position = v3fModule(mod);

            ship->power_delta = 0.0f;
            LOGFMT("Unit %u prevented the power surge from damaging ship %i.",
                   unit->id, ship_index);
            break;
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

      if (v3fDsq(unit->transform.position, v3fModule(mod)) < kDsqOperate)
        satisfied |= FLAG(mod->mkind);
    }
  }

  ship->operate_flags = satisfied;
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
ThinkMissle(uint64_t ship_index)
{
  if (!kScenario.missile) return;

  for (int i = 0; i < kUsedMissile; ++i) {
    Missile* missile = &kMissile[i];
    v2i tilepos = WorldToTilePos(missile->transform.position.xy());
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
ThinkPod(uint64_t ship_index)
{
  if (!kScenario.pod) return;

  for (int i = 0; i < kUsedPod; ++i) {
    Pod* pod = &kPod[i];
    if (pod->ship_index != ship_index) continue;
    // Keep-state on AiReturn
    constexpr uint64_t keep = (FLAG(kPodAiReturn) | FLAG(kPodAiUnmanned));
    const uint64_t keep_state = pod->think_flags & keep;
    v2f goal;
    uint64_t think_flags = 0;

    // Goal is to return home, unless overidden
    for (int k = 0; k < kUsedModule; ++k) {
      Module* mod = &kModule[k];
      if (mod->mkind != kModMine) continue;
      goal = TilePosToWorld(v2i(mod->cx, mod->cy));
      break;
    }

    if (kShip[ship_index].sys[kModMine] < .5f) {
      think_flags |= FLAG(kPodAiLostControl);
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
      float dsq;
      uint64_t asteroid_index = v3fNearTransform(
          pod->transform.position, GAME_ITER(Asteroid, transform), &dsq);

      if (asteroid_index < kUsedAsteroid) {
        Asteroid* asteroid = &kAsteroid[asteroid_index];
        think_flags |= FLAG(kPodAiApproach);
        goal = asteroid->transform.position.xy();
        if (dsq < 900.f) {
          think_flags |= FLAG(kPodAiGather);
          asteroid->deplete = 1;
        }
      } else {
        for (int i = 0; i < kUsedShip; ++i) {
          if (i == ship_index) continue;
          uint64_t grid_index = kShip[i].grid_index;
          v3f midship =
              kGrid[i].transform.position +
              v2f(kTileWidth * kMapWidth * .5f, kTileHeight * kMapHeight * .5f);
          dsq = v3fDsq(midship, pod->transform.position);
          if (dsq > kDsqOperate) {
            goal = midship.xy();
            think_flags |= FLAG(kPodAiApproach);
          } else {
            think_flags |= FLAG(kPodAiDisembark);
          }
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
  ThinkAsteroid();

  for (uint64_t i = 0; i < kUsedShip; ++i) {
    TilemapSet(kShip[i].grid_index);
    // Shroud is reset each frame
    TilemapUpdate(kScenario.tilemap);

    ThinkShip(i);
    ThinkMissle(i);
    ThinkPod(i);
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
  if (ship->think_flags & FLAG(kShipAiSpawnPod)) {
    Pod* pod = UsePod();
    pod->ship_index = ship_index;
    LOGFMT("Ship %u constructed a new pod %u", ship_index, pod - kPod);
    pod->transform = Transform{.position = v3f(520.f, 600.f, 0.f)};
    pod->think_flags = FLAG(kPodAiUnmanned);
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

void
DecidePod(uint64_t ship_index)
{
  if (!kScenario.pod) return;

  for (int i = 0; i < kUsedPod; ++i) {
    Pod* pod = &kPod[i];
    if (pod->ship_index != ship_index) continue;

    uint64_t action = TZCNT(pod->think_flags);
    uint64_t mineral = 0;
    v3f dir = v3f();
    switch (action) {
      case kPodAiLostControl:
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
        kResource[0].mineral += MIN(5, pod->mineral);
        pod->mineral -= MIN(5, pod->mineral);
        break;
      case kPodAiUnmanned:
        break;
      case kPodAiDisembark:
        for (int i = 0; i < kUsedUnit; ++i) {
          if (kUnit[i].inspace) {
            LOGFMT("%d disembarked onto ship %d", i, 1);
            kUnit[i].transform.position = pod->transform.position;
            kUnit[i].inspace = 0;
            kUnit[i].ship_index = 1;
            pod->think_flags = FLAG(kPodAiUnmanned);
            break;
          }
        }
        *kPod = kZeroPod;
        continue;
    };

#ifdef DEBUG_AI
    printf(" [ position %04.02fx %04.02fy ] ", pod->transform.position.x,
           pod->transform.position.y);
    printf(" [ goal %04.02fx %04.02fy ] ", pod->goal.x, pod->goal.y);
    printf("pod ai [ action %lu ] [ think %lu ] [dir %f %f]\n", action,
           pod->think_flags, dir.x, dir.y);
#endif

    if (!std::isnormal(dir.x)) dir.x = FP_ZERO;
    if (!std::isnormal(dir.y)) dir.y = FP_ZERO;
    pod->transform.position += dir * 2.0;
    pod->last_heading = dir.xy();
  }
}

bool
MoveTowards(Unit* unit, v2i tilepos, v3f dest, UnitAction set_on_arrival)
{
  v2i end = WorldToTilePos(dest);
  auto* path = PathTo(tilepos, end);
  if (!path) {
    unit->uaction = set_on_arrival;
    BB_REM(unit->bb, kUnitDestination);
    return true;
  }

  bool near_goal = (path->size == 1);
  v3f move_dir = TileAvoidWalls(tilepos) * !near_goal;
  move_dir *= kAvoidanceScaling;
  if (v3fDsq(unit->transform.position, dest) < 1.f) {
    unit->transform.position = dest;
    unit->uaction = set_on_arrival;
    BB_REM(unit->bb, kUnitDestination);
    return true;
  }

  v3f new_dest =
      (dest * near_goal) + (TilePosToWorld(path->tile[1]) * !near_goal);
  move_dir += math::Normalize(new_dest.xy() - unit->transform.position.xy()) *
              unit->speed;
  unit->transform.position += move_dir;
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
  if (kResource[0].frame - unit->attack_frame < unit->attack_cooldown) {
    return;
  }

  ProjectileCreate(target, unit, 7.5f, unit->weapon_kind);
  unit->attack_frame = kResource[0].frame;
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
      v2i tilepos(m->cx, m->cy);
      v3f world = TilePosToWorld(tilepos);
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

  // Two at a time (one crew, one enemy)
  int team_count[kAllianceCount] = {};
  for (int i = 0; i < kUsedUnit; ++i) {
    team_count[kUnit[i].alliance]++;
  }
  Alliance reinforce_team =
      team_count[kCrew] < team_count[kEnemy] ? kCrew : kEnemy;
  if (kUsedUnit < kMaxUnit) {
    for (int i = 0; i < kUsedModule; ++i) {
      Module* m = &kModule[i];
      if (m->ship_index != ship_index) continue;
      if (m->mkind != kModBarrack) continue;
      // Hack: Module 0 spawns kCrew, others spawn kEnemy
      if (i != reinforce_team) continue;
      v2f random_dir = Normalize(
          TileRandomPosition() - TilePosToWorld(v2i(m->cx, m->cy)));
      Unit* unit = UseIdUnit();
      unit->transform.position =
          TilePosToWorld(v2i(m->cx, m->cy)) + (random_dir * kTileWidth);
      unit->transform.scale = v3f(0.25f, 0.25f, 0.f);
      uint8_t attrib[CREWA_MAX] = {11, 10, 11, 10};
      memcpy(unit->acurrent, attrib, sizeof(attrib));
      unit->ship_index = ship_index;
      unit->kind = kOperator;
      unit->alliance = reinforce_team;
      unit->player_id = AssignPlayerId();
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
    v2i tilepos = WorldToTilePos(transform->position.xy());
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
                tile_world_distance * tile_world_distance);
    }

    // Crew has been sucked away into the vacuum
    if (tile->nooxygen) {
      unit->uaction = kUaVacuum;
    }

    AIThink(unit);

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
        kResource[0].mineral += c->minerals;
        *c = kZeroConsumable;
      }
    }
  }
}

void
Decide()
{
  uint64_t units_selected = CountUnitSelection(kPlayerIndex);
  while (CountCommand()) {
    Command c = PopCommand();
    if (c.unit_id == kInvalidUnit) {
      bool group_move = units_selected > 1 && c.type == kUaMove;
      const unsigned player_control = (1 << kPlayerIndex);
      TilemapSet(TilemapWorldToGrid(c.destination));
      BfsStart(WorldToTilePos(c.destination));
      for (int i = 0; i < kUsedUnit; ++i) {
        // The issuer of a command must have a set bit
        if (0 == (kUnit[i].control & c.control)) continue;
        if (group_move) {
          v2i pos;
          BfsNext(&pos);
          c.destination = TilePosToWorld(pos);
        }
        ApplyCommand(&kUnit[i], c);
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

  for (uint64_t i = 0; i < kUsedShip; ++i) {
    TilemapSet(kShip[i].grid_index);
    DecideShip(i);
    DecideMissle(i);
    DecidePod(i);
    UpdateModule(i);
    UpdateUnit(i);
    UpdateConsumable(i);
  }
}

void
Update()
{
  kSimulationOver = ScenarioOver();
  ++kResource[0].frame;

  // Camera can move even when game is over
  for (int i = 0; i < kUsedPlayer; ++i) {
    camera::Update(&kPlayer[i].camera);
    kPlayer[i].camera.motion.z = 0.f;
  }

  if (kSimulationOver) return;

  for (int i = 0; i < kUsedShip; ++i) {
    TilemapSet(kShip[i].grid_index);
  }

  Think();
  Decide();
  TilemapSet(-1);

  for (int i = 0; i < kUsedNotify; ++i) {
    Notify* n = &kNotify[i];
    if (POPCNT(n->age) == kNotifyAgeBits) {
      *n = kZeroNotify;
    } else {
      n->age += (n->age > 0);
    }
  }

  ProjectileSimulation();

  RegistryCompact();
}  // namespace simulation

}  // namespace simulation
