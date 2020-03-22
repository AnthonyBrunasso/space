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
  }
  return v3f();
}

v3f
ModulePosition(Module* mod)
{
  return TilePosToWorld(v2i(mod->cx, mod->cy));
}

bool
ModuleNear(Module* module, v3f loc)
{
  // TODO: Take into consideration module bounds.
  if (LengthSquared(loc - ModulePosition(module)) < 50.f * 35.f) return true;
  return false;
}


}  // namespace simulation
