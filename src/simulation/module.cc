#pragma once

namespace simulation {

bool
ModuleBuilt(Module* module)
{
  if (!module) return false;
  return module->frames_building >= module->frames_to_build;
}

void
ModuleSetBuilt(Module* module)
{
  if (!module) return;
  module->frames_building = module->frames_to_build;
}

uint64_t
ModuleCost(ModuleKind mkind)
{
  switch (mkind) {
    case kModMine:
      return 400;
    case kModBarrack:
      return 400;
    case kModMedbay:
      return 400;
    case kModWarp:
      return 30;
    case kModTurret:
    case kModEngine:
    case kModPower:
    case kModCount:
    default:
      return 0;
  }
  return 0;
}

const char*
ModuleName(ModuleKind mkind)
{
  switch (mkind) {
    case kModMine:
      return "Mining Drill";
    case kModBarrack:
      return "Barracks";
    case kModMedbay:
      return "Medbay";
    case kModTurret:
      return "Turret";
    case kModEngine:
      return "Engine";
    case kModPower:
      return "Power";
    case kModWarp:
      return "Warp";
    case kModCount:
    default:
      return "Unknown";
  }
  return "Unknown";

}

// TODO: Probably add some grid checking here.
bool
ModuleCanBuild(ModuleKind mkind, Player* player) {
  if (!player) return false;
  return player->resource.mineral >= ModuleCost(mkind);
}

v3f
ModuleColor(ModuleKind mkind)
{
  switch (mkind) {
    case kModPower:
      return v3f(0.0f, 0.0f, 0.75f);
    case kModEngine:
      return v3f(1.0f, 0.0f, 1.f);
    case kModTurret:
      return v3f(0.3f, 0.3f, 0.0f);
    case kModMine:
      return v3f(0.0, 0.75f, 0.0f);
    case kModBarrack:
      return v3f(1.0, 0.0, 0.0);
    case kModMedbay:
      return v3f(1.0, 1.0, 1.0);
    case kModWarp:
      return v3f(0.6, 1.0, 1.0);
    case kModCount:
    default:
      return v3f();

  }
  return v3f();
}

v3f
ModuleBounds(ModuleKind mkind)
{
  switch (mkind) {
    case kModPower:
      return v3f(15.f, 15.f, 15.f);
    case kModEngine:
      return v3f(15.f, 15.f, 15.f);
    case kModTurret:
      return v3f(15.f, 15.f, 15.f);
    case kModMine:
      return v3f(15.f, 15.f, 15.f);
    case kModBarrack:
      return v3f(45.f, 45.f, 15.f);
    case kModMedbay:
      return v3f(45.f, 45.f, 15.f);
    case kModWarp:
      return v3f(15.f, 15.f, 15.f);
    case kModCount:
    default:
      return v3f();
  }
  return v3f();
}

v3f
ModulePosition(Module* mod)
{
  TilemapModify modify(mod->ship_index);
  return TilePosToWorld(mod->tile);
}

bool
ModuleNear(Module* module, v3f loc)
{
  // TODO: Take into consideration module bounds.
  if (LengthSquared(loc - ModulePosition(module)) < 50.f * 38.f) return true;
  return false;
}

void
ModuleMineUpdate(Module* module)
{
  // Find the nearest asteroid
  float d = FLT_MAX;
  v3f p;
  Asteroid* a = nullptr;
  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    if (TilemapWorldToGrid(asteroid->transform.position) !=
        module->ship_index) {
      continue;
    }
    float nd = math::LengthSquared(
          asteroid->transform.position - ModulePosition(module));
    if (nd < d) {
      d = nd;
      p = asteroid->transform.position;
      a = asteroid;
    }
  }

  if (!a) return;

  ProjectileCreate(p, ModulePosition(module), 15.f, 2,
                   kWeaponMiningLaser);
  kPlayer[module->player_id].resource.mineral += .1f;
}

void
ModuleBarrackUpdate(Module* module)
{
  Player* player = &kPlayer[module->player_id];

  if (module->frames_training != kTrainIdle) {
    ++module->frames_training;
  }

  if (module->frames_training >= module->frames_to_train) {
    SpawnCrew(module->tile, module->ship_index);
    module->frames_training = kTrainIdle;
  }

  // Barracks automatically trains a unit if it has enough minerals, it's
  // enabled and it's training state is not idle.
  if (module->frames_training != kTrainIdle || !module->enabled ||
      player->resource.mineral < 50.f) return;

  player->resource.mineral -= 50.f;
  module->frames_training = 0;
}

void
ModuleWarpUpdate(Module* module)
{
  Unit* unit = GetNearestUnit(TilePosToWorld(module->tile));
  v2i ut;
  if (!WorldToTilePos(unit->transform.position, &ut)) return;
  if (ut != module->tile) {
    module->frames_training = kTrainIdle;
    return;
  }
  // Find the nearest warp on a seperate tilemap from this one.
  Module* tm = nullptr;
  float d = FLT_MAX;
  for (int i = 0; i < kUsedModule; ++i) {
    Module* nm = &kModule[i];
    if (nm == module) continue;
    if (!ModuleBuilt(nm)) continue;
    if (nm->ship_index == module->ship_index) continue;
    if (nm->mkind != kModWarp) continue;
    // Target warp must be idle.
    if (nm->frames_training != kTrainIdle) continue;
    float nd = LengthSquared(nm->tile - module->tile);
    if (nd < d) {
      tm = nm;
      d = nd;
    }
  }
  if (!tm) return;

  if (module->frames_training == kTrainIdle) {
    module->frames_training = 0;
  } else {
    ++module->frames_training;
  }

  if (module->frames_training >= module->frames_to_train) {
    LOGFMT("Warping crew to ship %i", tm->ship_index);
    module->frames_training = kTrainIdle;
    TilemapModify mod(tm->ship_index);
    unit->transform.position = TilePosToWorld(tm->tile);
    unit->ship_index = tm->ship_index;
  }
}

void
ModuleUpdate(Module* module)
{
  switch (module->mkind) {
    case kModMine: {
      ModuleMineUpdate(module);
    } break;
    case kModBarrack: {
      ModuleBarrackUpdate(module);
    } break;
    case kModWarp: {
      ModuleWarpUpdate(module);
    } break;
    default: break;
  }
}

}  // namespace simulation
