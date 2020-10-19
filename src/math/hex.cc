#pragma once

#include <vector>

namespace math {

static const r32 kSqrt3 = sqrt(3.f);
static const r32 kSqrt3Div2 = kSqrt3 / 2.f;

static v2f kHexAxialNeighbors[6] = {
  {1.f, 0.f}, {0.f, 1.f}, {-1.f, 1.f}, {-1.f, 0.f}, {0.f, -1.f}, {1.f, -1.f}
};

static v3f kHexCubeNeighbors[6] = {
  {1.f,0.f,-1.f}, {0.f,1.f,-1.f}, {-1.f,1.f,0.f},
  {-1.f,0.f,1.f}, {0.f,-1.f,1.f}, {1.f,-1.f,0.f},
};

v2f
HexCorner(const v2f& center, r32 size, s32 corner)
{
  assert(corner <= 6);
  return center + Rotate(v2f(size, 0.f), corner * 60.f - 30.f);
}

v2f
HexCubeToAxial(const v3f& cube)
{
  return v2f(cube.x, cube.z);
}

v3f
HexAxialToCube(const v2f& axial)
{
  return v3f(axial.x, axial.y, -axial.x - axial.y);
}

v2f
HexAxialToWorld(const v2f& axial, r32 size)
{
  return v2f(size * (kSqrt3 * axial.x + kSqrt3Div2 * axial.y),
             size * (3.f / 2.f * axial.y));
}

v2f
HexAxialNeighbor(const v2f& axial, s32 i)
{
  assert(i >= 0 && i < 6);
  return axial + kHexAxialNeighbors[i];
}

v3f
HexCubeNeighbor(const v3f& cube, s32 i)
{
  assert(i >= 0 && i < 6);
  return cube + kHexCubeNeighbors[i];
}

r32
HexCubeDistance(const v3f& a, const v3f& b)
{
  return (abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z)) / 2.f;
}

r32
HexAxialDistance(const v2f& a, const v2f& b)
{
  return HexCubeDistance(HexAxialToCube(a), HexAxialToCube(b));
}

std::vector<v2f>
HexAxialRange(s32 n)
{
  std::vector<v2f> range;
  for (int x = -n; x <= n; ++x) {
    for (int y = -n; y <= n; ++y) {
      for (int z = -n; z <= n; ++z) {
        if (x + y + z == 0) {
          range.push_back(HexCubeToAxial(v3f(x, y, z)));
        }
      }
    }
  }
  return range;
}

std::vector<v3f>
HexCubeRange(s32 n)
{
  std::vector<v3f> range;
  for (int x = -n; x <= n; ++x) {
    for (int y = -n; y <= n; ++y) {
      for (int z = -n; z <= n; ++z) {
        if (x + y + z == 0) {
          range.emplace_back(x, y, z);
        }
      }
    }
  }
  return range;
}

}  // namespace math
