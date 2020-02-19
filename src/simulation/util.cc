
#include "math/vec.h"

#include "entity.cc"
#include "tilemap.cc"

#include <cstdint>

namespace simulation
{
v3f
v3fModule(Module* mod)
{
  return TilePosToWorld(v2i(mod->cx, mod->cy));
}

float
v3fDsq(const v3f& dst, const v3f& src)
{
  v3f delta = dst - src;
  return LengthSquared(delta);
}

uint64_t
v3fNear(v3f pos, uint64_t step, const uint8_t* start, const uint8_t* end)
{
  uint64_t index = UINT64_MAX;
  float nearest = FLT_MAX;
  int i = 0;
  for (const uint8_t* iter = start; iter < end; iter += step, i += 1) {
    const Transform* t = (const Transform*)iter;
    float distance = v3fDsq(pos, t->position);
    if (distance < nearest) {
      nearest = distance;
      index = i;
    }
  }

  return index;
}

}  // namespace simulation
