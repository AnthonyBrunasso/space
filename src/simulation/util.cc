
#include "math/vec.h"

#include "entity.cc"
#include "tilemap.cc"

namespace simulation
{
v3f
v3fModule(Module* mod)
{
  return TilePosToWorld(v2i(mod->cx, mod->cy));
}

float
v3fDsq(v3f dst, v3f src)
{
  v3f delta = dst - src;
  return delta.x * delta.x + delta.y * delta.y;
}
}  // namespace simulation
