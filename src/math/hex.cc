#pragma once

namespace math {

static const r32 kSqrt3 = sqrt(3.f);
static const r32 kSqrt3Div2 = kSqrt3 / 2.f;

v2f
HexCorner(const v2f& center, r32 size, s32 corner)
{
  assert(corner <= 6);
  return center + Rotate(v2f(size, 0.f), corner * 60.f - 30.f);
}

v2f
CubeToAxial(const v3f& cube)
{
  return v2f(cube.x, cube.z);
}

v3f
AxialToCube(const v2f& axial)
{
  return v3f(axial.x, axial.y, -axial.x - axial.y);
}

v2f
AxialToWorld(const v2f& axial, r32 size)
{
  return v2f(size * (kSqrt3 * axial.x + kSqrt3Div2 * axial.y),
             size * (3.f / 2.f * axial.y));
}

}  // namespace math
