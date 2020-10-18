#pragma once

namespace math {

v2f
HexCorner(const v2f& center, r32 size, s32 corner)
{
  assert(corner <= 6);
  return Rotate(center + v2f(size, 0.f), corner * 60.f - 30.f);
}

}  // namespace math
