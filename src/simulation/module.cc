#pragma once

namespace simulation {

bool
ModuleBuilt(Module* module)
{
  if (!module) return false;
  return module->frames_progress >= module->frames_to_complete;
}

void
ModuleSetBuilt(Module* module)
{
  if (!module) return;
  module->frames_progress = module->frames_to_complete;
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
    case kModTurret:
    case kModEngine:
    case kModPower:
    case kModCount:
    default:
      return 0;
  }
  return 0;
}

char*
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


}  // namespace simulation
